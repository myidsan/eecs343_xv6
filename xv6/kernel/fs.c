// File system implementation.  Four layers:
//   + Blocks: allocator for raw disk blocks.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
//
// Disk layout is: superblock, inodes, block in-use bitmap, data blocks.
//
// This file contains the low-level file system manipulation 
// routines.  The (higher-level) system call implementations
// are in sysfile.c.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "buf.h"
#include "fs.h"
#include "file.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
static void itrunc(struct inode*);

// Read the super block.
static void
readsb(int dev, struct superblock *sb)
{
  struct buf *bp;
  
  bp = bread(dev, 1);
  memmove(sb, bp->data, sizeof(*sb));
  brelse(bp);
}

// Zero a block.
static void
bzero(int dev, int bno)
{
  struct buf *bp;
  
  bp = bread(dev, bno);
  memset(bp->data, 0, BSIZE);
  bwrite(bp);
  brelse(bp);
}

// Blocks. 

// Allocate a disk block.
static uint
balloc(uint dev)
{
  int b, bi, m;
  struct buf *bp;
  struct superblock sb;

  bp = 0;
  readsb(dev, &sb);
  for(b = 0; b < sb.size; b += BPB){
    bp = bread(dev, BBLOCK(b, sb.ninodes));
    for(bi = 0; bi < BPB; bi++){
      m = 1 << (bi % 8);
      if((bp->data[bi/8] & m) == 0){  // Is block free?
        bp->data[bi/8] |= m;  // Mark block in use on disk.
        bwrite(bp);
        brelse(bp);
        return b + bi;
      }
    }
    brelse(bp);
  }
  panic("balloc: out of blocks");
}

// Free a disk block.
static void
bfree(int dev, uint b)
{
  struct buf *bp;
  struct superblock sb;
  int bi, m;

  bzero(dev, b);

  readsb(dev, &sb);
  bp = bread(dev, BBLOCK(b, sb.ninodes));
  bi = b % BPB;
  m = 1 << (bi % 8);
  if((bp->data[bi/8] & m) == 0)
    panic("freeing free block");
  bp->data[bi/8] &= ~m;  // Mark block free on disk.
  bwrite(bp);
  brelse(bp);
}

// Inodes.
//
// An inode is a single, unnamed file in the file system.
// The inode disk structure holds metadata (the type, device numbers,
// and data size) along with a list of blocks where the associated
// data can be found.
//
// The inodes are laid out sequentially on disk immediately after
// the superblock.  The kernel keeps a cache of the in-use
// on-disk structures to provide a place for synchronizing access
// to inodes shared between multiple processes.
// 
// ip->ref counts the number of pointer references to this cached
// inode; references are typically kept in struct file and in proc->cwd.
// When ip->ref falls to zero, the inode is no longer cached.
// It is an error to use an inode without holding a reference to it.
//
// Processes are only allowed to read and write inode
// metadata and contents when holding the inode's lock,
// represented by the I_BUSY flag in the in-memory copy.
// Because inode locks are held during disk accesses, 
// they are implemented using a flag rather than with
// spin locks.  Callers are responsible for locking
// inodes before passing them to routines in this file; leaving
// this responsibility with the caller makes it possible for them
// to create arbitrarily-sized atomic operations.
//
// To give maximum control over locking to the callers, 
// the routines in this file that return inode pointers 
// return pointers to *unlocked* inodes.  It is the callers'
// responsibility to lock them before using them.  A non-zero
// ip->ref keeps these unlocked inodes in the cache.

struct {
  struct spinlock lock;
  struct inode inode[NINODE];
} icache;

void
iinit(void)
{
  initlock(&icache.lock, "icache");
}

static struct inode* iget(uint dev, uint inum);

// Allocate a new inode with the given type on device dev.
struct inode*
ialloc(uint dev, short type)
{
  int inum;
  struct buf *bp;
  struct dinode *dip;
  struct superblock sb;

  readsb(dev, &sb);
  for(inum = 1; inum < sb.ninodes; inum++){  // loop over inode blocks
    bp = bread(dev, IBLOCK(inum));
    dip = (struct dinode*)bp->data + inum%IPB;
    if(dip->type == 0){  // a free inode
      memset(dip, 0, sizeof(*dip));
      dip->type = type;
      bwrite(bp);   // mark it allocated on the disk
      brelse(bp);
      return iget(dev, inum);
    }
    brelse(bp);
  }
  panic("ialloc: no inodes");
}

// Copy inode, which has changed, from memory to disk.
void
iupdate(struct inode *ip)
{
  struct buf *bp;
  struct dinode *dip;

  bp = bread(ip->dev, IBLOCK(ip->inum));
  dip = (struct dinode*)bp->data + ip->inum%IPB;
  dip->type = ip->type;
  dip->major = ip->major;
  dip->minor = ip->minor;
  dip->nlink = ip->nlink;
  dip->size = ip->size;
  memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
  bwrite(bp);
  brelse(bp);
}

// Find the inode with number inum on device dev
// and return the in-memory copy.
static struct inode*
iget(uint dev, uint inum)
{
  struct inode *ip, *empty;

  acquire(&icache.lock);

  // Try for cached inode.
  empty = 0;
  for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
    if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
      ip->ref++;
      release(&icache.lock);
      return ip;
    }
    if(empty == 0 && ip->ref == 0)    // Remember empty slot.
      empty = ip;
  }

  // Allocate fresh inode.
  if(empty == 0)
    panic("iget: no inodes");

  ip = empty;
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->flags = 0;
  release(&icache.lock);

  return ip;
}

// Increment reference count for ip.
// Returns ip to enable ip = idup(ip1) idiom.
struct inode*
idup(struct inode *ip)
{
  acquire(&icache.lock);
  ip->ref++;
  release(&icache.lock);
  return ip;
}

// Lock the given inode.
void
ilock(struct inode *ip)
{
  struct buf *bp;
  struct dinode *dip;

  if(ip == 0 || ip->ref < 1)
    panic("ilock");

  acquire(&icache.lock);
  while(ip->flags & I_BUSY)
    sleep(ip, &icache.lock);
  ip->flags |= I_BUSY;
  release(&icache.lock);

  if(!(ip->flags & I_VALID)){
    bp = bread(ip->dev, IBLOCK(ip->inum));
    dip = (struct dinode*)bp->data + ip->inum%IPB;
    ip->type = dip->type;
    ip->major = dip->major;
    ip->minor = dip->minor;
    ip->nlink = dip->nlink;
    ip->size = dip->size;
    memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
    brelse(bp);
    ip->flags |= I_VALID;
    if(ip->type == 0)
      panic("ilock: no type");
  }
}

// Unlock the given inode.
void
iunlock(struct inode *ip)
{
  if(ip == 0 || !(ip->flags & I_BUSY) || ip->ref < 1)
    panic("iunlock");

  acquire(&icache.lock);
  ip->flags &= ~I_BUSY;
  wakeup(ip);
  release(&icache.lock);
}

// Caller holds reference to unlocked ip.  Drop reference.
void
iput(struct inode *ip)
{
  acquire(&icache.lock);
  if(ip->ref == 1 && (ip->flags & I_VALID) && ip->nlink == 0){
    // inode is no longer used: truncate and free inode.
    if(ip->flags & I_BUSY)
      panic("iput busy");
    ip->flags |= I_BUSY;
    release(&icache.lock);
    itrunc(ip);
    ip->type = 0;
    iupdate(ip);
    acquire(&icache.lock);
    ip->flags = 0;
    wakeup(ip);
  }
  ip->ref--;
  release(&icache.lock);
}

// Common idiom: unlock, then put.
void
iunlockput(struct inode *ip)
{
  iunlock(ip);
  iput(ip);
}

// Inode contents
//
// The contents (data) associated with each inode is stored
// in a sequence of blocks on the disk.  The first NDIRECT blocks
// are listed in ip->addrs[].  The next NINDIRECT blocks are 
// listed in the block ip->addrs[NDIRECT].

// Return the disk block address of the nth block in inode ip.
// If there is no such block, bmap allocates one.
static uint
bmap(struct inode *ip, uint bn)
{
  uint addr, *a;
  struct buf *bp;

  if(bn < NDIRECT){
    if((addr = ip->addrs[bn]) == 0)
      ip->addrs[bn] = addr = balloc(ip->dev);
    return addr;
  }
  bn -= NDIRECT;

  if(bn < NINDIRECT){
    // Load indirect block, allocating if necessary.
    if((addr = ip->addrs[NDIRECT]) == 0)
      ip->addrs[NDIRECT] = addr = balloc(ip->dev);
    bp = bread(ip->dev, addr);
    a = (uint*)bp->data;
    if((addr = a[bn]) == 0){
      a[bn] = addr = balloc(ip->dev);
      bwrite(bp);
    }
    brelse(bp);
    return addr;
  }

  panic("bmap: out of range");
}

// Truncate inode (discard contents).
// Only called after the last dirent referring
// to this inode has been erased on disk.
static void
itrunc(struct inode *ip)
{
  int i, j;
  struct buf *bp;
  uint *a;

  for(i = 0; i < NDIRECT; i++){
    if(ip->addrs[i]){
      bfree(ip->dev, ip->addrs[i]);
      ip->addrs[i] = 0;
    }
  }
  
  if(ip->addrs[NDIRECT]){
    bp = bread(ip->dev, ip->addrs[NDIRECT]);
    a = (uint*)bp->data;
    for(j = 0; j < NINDIRECT; j++){
      if(a[j])
        bfree(ip->dev, a[j]);
    }
    brelse(bp);
    bfree(ip->dev, ip->addrs[NDIRECT]);
    ip->addrs[NDIRECT] = 0;
  }

  ip->size = 0;
  iupdate(ip);
}

// Copy stat information from inode.
void
stati(struct inode *ip, struct stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
}

// Read data from inode.
int
readi(struct inode *ip, char *dst, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
      return -1;
    return devsw[ip->major].read(ip, dst, n);
  }

  if(off > ip->size || off + n < off)
    return -1;
  if(off + n > ip->size)
    n = ip->size - off;

  for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
    bp = bread(ip->dev, bmap(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(dst, bp->data + off%BSIZE, m);
    brelse(bp);
  }
  return n;
}

// Write data to inode.
int
writei(struct inode *ip, char *src, uint off, uint n)
{
  uint tot, m;
  struct buf *bp;

  if(ip->type == T_DEV){
    if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
      return -1;
    return devsw[ip->major].write(ip, src, n);
  }

  if(off > ip->size || off + n < off)
    return -1;
  if(off + n > MAXFILE*BSIZE)
    n = MAXFILE*BSIZE - off;

  for(tot=0; tot<n; tot+=m, off+=m, src+=m){
    bp = bread(ip->dev, bmap(ip, off/BSIZE));
    m = min(n - tot, BSIZE - off%BSIZE);
    memmove(bp->data + off%BSIZE, src, m);
    bwrite(bp);
    brelse(bp);
  }

  if(n > 0 && off > ip->size){
    ip->size = off;
    iupdate(ip);
  }
  return n;
}

// Directories

int
namecmp(const char *s, const char *t)
{
  return strncmp(s, t, DIRSIZ);
}

// Look for a directory entry in a directory.
// If found, set *poff to byte offset of entry.
// Caller must have already locked dp.
struct inode*
dirlookup(struct inode *dp, char *name, uint *poff)
{
  uint off, inum;
  struct buf *bp;
  struct dirent *de;

  if(dp->type != T_DIR)
    panic("dirlookup not DIR");

  for(off = 0; off < dp->size; off += BSIZE){
    bp = bread(dp->dev, bmap(dp, off / BSIZE));
    for(de = (struct dirent*)bp->data;
        de < (struct dirent*)(bp->data + BSIZE);
        de++){
      if(de->inum == 0)
        continue;
      if(namecmp(name, de->name) == 0){
        // entry matches path element
        if(poff)
          *poff = off + (uchar*)de - bp->data;
        inum = de->inum;
        brelse(bp);
        return iget(dp->dev, inum);
      }
    }
    brelse(bp);
  }
  return 0;
}

// Write a new directory entry (name, inum) into the directory dp.
int
dirlink(struct inode *dp, char *name, uint inum)
{
  int off;
  struct dirent de;
  struct inode *ip;

  // Check that name is not present.
  if((ip = dirlookup(dp, name, 0)) != 0){
    iput(ip);
    return -1;
  }

  // Look for an empty dirent.
  for(off = 0; off < dp->size; off += sizeof(de)){
    if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("dirlink read");
    if(de.inum == 0)
      break;
  }

  strncpy(de.name, name, DIRSIZ);
  de.inum = inum;
  if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("dirlink");
  
  return 0;
}

// Paths

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char*
skipelem(char *path, char *name)
{
  char *s;
  int len;

  while(*path == '/')
    path++;
  if(*path == 0)
    return 0;
  s = path;
  while(*path != '/' && *path != 0)
    path++;
  len = path - s;
  if(len >= DIRSIZ)
    memmove(name, s, DIRSIZ);
  else {
    memmove(name, s, len);
    name[len] = 0;
  }
  while(*path == '/')
    path++;
  return path;
}

// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
static struct inode*
namex(char *path, int nameiparent, char *name)
{
  struct inode *ip, *next;

  if(*path == '/')
    ip = iget(ROOTDEV, ROOTINO);
  else
    ip = idup(proc->cwd);

  while((path = skipelem(path, name)) != 0){
    ilock(ip);
    if(ip->type != T_DIR){
      iunlockput(ip);
      return 0;
    }
    if(nameiparent && *path == '\0'){
      // Stop one level early.
      iunlock(ip);
      return ip;
    }
    if((next = dirlookup(ip, name, 0)) == 0){
      iunlockput(ip);
      return 0;
    }
    iunlockput(ip);
    ip = next;
  }
  if(nameiparent){
    iput(ip);
    return 0;
  }
  return ip;
}

struct inode*
namei(char *path)
{
  char name[DIRSIZ];
  return namex(path, 0, name);
}

struct inode*
nameiparent(char *path, char *name)
{
  return namex(path, 1, name);
}

// search for the given key in data block
// Enforcing maximun length on the tag value of 18 bytes
// Therfore total 16 tag structs
int 
searchKey(uchar* key, uchar* str)
{
	int i = 0, j = 0;
	int keyLength = strlen((char*)key);
	for(i = 0; i < BSIZE; i += 32) {
		for(j = 0; j < 10 && i + j < BSIZE && key[j] == str[i + j]; ++j) {
	    if(j == keyLength && !key[j] && !str[i+j]) {
        cprintf("found key\n\n");
        return i + j - keyLength;	
      }
    }
	}
	return -1;
}

// search for the end of tag block
// if it exceeds block size (BSZIE) return -1
int 
searchEnd(uchar* str) 
{
	int i;
	for(i = 0; str[i] && i < BSIZE; i += 32);
	if (i == BSIZE) 
		return -1;
  return i;
}

int
tagFile(int fileDescriptor, char* key, char* value, int valueLength)
{
  struct file *f;
  struct buf *buftag;
  uchar *str;
  //cprintf("1\n");

  // checks if fileDescriptor is valid and is open.
  if(fileDescriptor < 0 || fileDescriptor >= NOFILE || (f = proc->ofile[fileDescriptor]) == 0) 
    return -1;
  //cprintf("2\n");
  // checks if file is inode, writeable, and has inode called ip
  if(f->type != FD_INODE || !f->writable || !f->ip)
    return -1;
  //cprintf("3\n");
  // checks keyLength
  int keyLength = strlen(key);
  if(!key || keyLength < 1 || keyLength > 9)
    return -1;
  //cprintf("4\n");
  // checks value and value length
  if(!value || valueLength < 0 || valueLength > 18)
    return -1;
  //cprintf("5\n");
  // lock inode
  ilock(f->ip);
  //cprintf("6\n");
  if (!f->ip->tags){
    cprintf("allocating since first time\n");
    f->ip->tags = balloc(f->ip->dev); // allocate a disk block
  }
  //cprintf("7\n");
	  
  buftag = bread(f->ip->dev, f->ip->tags); // To get a buffer for a particular disk block,call bread
  //cprintf("9\n");
  str = (uchar*)buftag->data; // limited to 512 in buf.h
  //cprintf("working till searchKey\n");
	int keyPosition = searchKey((uchar*)key, (uchar*)str);
  //cprintf("searchKey working\n");
	int endPosition = searchEnd((uchar*)str); 
  //cprintf("searchEnd working\n");
	
	// key is not found
  if(keyPosition < 0) {
		// no more space to put tags
		if(endPosition < 0) {
			brelse(buftag);
			iunlock(f->ip);
      cprintf("no more space\n");
			return -1;
		}
		// add new key and value to the allocated tag block
		// memset clears indicated bytes of within the block
		// memmove 
    cprintf("endPosition for creating new: %x\n", endPosition);
	  memset((void*)((uint)str + (uint)endPosition), 0, 28); // 10(key) + 18(value)	
	  memmove((void*)((uint)str + (uint)endPosition), (void*)key, (uint)keyLength); 	
	  memmove((void*)((uint)str + (uint)endPosition + 10), (void*)value, (uint)valueLength);
	} else {
	  // key is found. Update value
    cprintf("keyPosition for updating: %x\n", endPosition);
	  memset((void*)((uint)str + (uint)keyPosition + 10), 0, 18);
    memmove((void*)((uint)str + (uint)keyPosition + 10), (void*)value, (uint)valueLength); 	
	}
  bwrite(buftag);
  brelse(buftag);
	iunlock(f->ip);
  return 1;	
}

int 
removeFileTag(int fileDescriptor, char* key)
{
  struct file *f;
  struct buf *buftag;
  int keyLength = strlen(key);
  uchar *str;
  
  // checks if fileDescriptor is valid and is open.
  if(fileDescriptor < 0 || fileDescriptor >= NOFILE || (f = proc->ofile[fileDescriptor]) == 0) 
    return -1;
  // checks if file is inode, writeable, and has inode called ip
  if(f->type != FD_INODE || !f->writable || !f->ip || !f->ip->tags)
    return -1;
  // checks keyLength
  if(!key || keyLength < 1 || keyLength > 9)
    return -1;
  ilock(f->ip);
  buftag = bread(f->ip->dev, f->ip->tags);
  str = (uchar*)buftag->data;
         
	int keyPosition = searchKey((uchar*)key, (uchar*)str);
  if (keyPosition < 0) {
    brelse(buftag);
    iunlock(f->ip);
    return -1;
  } else {
    memset((void*)((uint)str + (uint)keyPosition), 0, 28);
    bwrite(buftag);
    brelse(buftag);
    iunlock(f->ip);
  }
  return 1;
}

int
getFileTag(int fileDescriptor, char* key, char* buffer, int length)
{
  struct file *f;
  struct buf *buftag;
  uchar *str;
  // checks if fileDescriptor is valid and is open.
  if(fileDescriptor < 0 || fileDescriptor >= NOFILE || (f = proc->ofile[fileDescriptor]) == 0) {
    cprintf("file descriptor is invalid\n");
    return -1;
  }
  if(f->type != FD_INODE || !f->writable || !f->ip) {
    cprintf("file is invalid\n");
    return -1;
  }
  // checks keyLength
  int keyLength = strlen(key);
  if (!key) {
    cprintf("where is my fucking key\n");
    return -1;
  }

  if(keyLength < 1) {
    cprintf("key input short\n");
    return -1;
  }

  if(keyLength > 9) {
    cprintf("key input long\n");
    return -1;
  }
  ilock(f->ip);
  if(!f->ip->tags) {
    cprintf("tag not allocated yet\n");
    iunlock(f->ip);
    return -1;
  }
  buftag = bread(f->ip->dev, f->ip->tags);
  str = (uchar*)buftag->data;
  //cprintf("keyPosition: %d", keyPosition);
  int keyPosition = searchKey((uchar*)key, (uchar*)str);
  cprintf("keyPosition is: %d\n", keyPosition);
  if(keyPosition < 0) {
    cprintf("key not found\n");
    brelse(buftag);
    iunlock(f->ip);
    return -1;
  } else {
    int i;
    uchar *found_key = (uchar *)((uint)str + (uint)keyPosition + 10);
    for(i = 0; (i < 18) && ((i < length) || found_key[i]); i++);
    if(i > length) {
      cprintf("%d\n", i);
      cprintf("error\n");
      brelse(buftag);
      iunlock(f->ip);
      return i;
    } else {
      cprintf("position of start: %x\n", keyPosition);
      memmove((void*)buffer, (void*)((uint)str + (uint)keyPosition + 10), i);
      brelse(buftag);
      iunlock(f->ip);
      return i;
    }
  }
}

int
getAllTags(int fileDescriptor, struct Key keys[], int maxTag)
{
  struct file *f;
  struct buf *buftag;
  uchar str[BSIZE];
  uint i = 0;
  int tagCount = 0;  

  // checks if fileDescriptor is valid and is open.
  if(fileDescriptor < 0 || fileDescriptor >= NOFILE || (f = proc->ofile[fileDescriptor]) == 0) 
    return -1;
  // checks if file is inode, writeable, and has inode called ip
  if(f->type != FD_INODE || !f->readable || !f->ip)
    return -1;
  // check if expected key list is empty 
  if(!keys)
    return -1;
  // maxTag must be larger than one
  if(maxTag < 0)
    return -1;
  
  //memset((void*)keys, 0, BSIZE);
  ilock(f->ip);
  if(!f->ip->tags)
    f->ip->tags = balloc(f->ip->dev);
  buftag = bread(f->ip->dev, f->ip->tags);
  memmove((void*)str, (void*)buftag->data, (uint)BSIZE); 
  brelse(buftag);
  iunlock(f->ip);
  for(i = 0; i < BSIZE; i+=32) {
    if(str[i]) {
      if(maxTag <= tagCount) {
        return -1;
      }
      cprintf("key is:%s\n", (char*)((uint)str+i));
      cprintf("key length:%x\n", (uint)strlen((char*)(uint)str+i));
      //memmove((void*)keys[tagCount].key, (void*)((uint)str + i), (uint)strlen((char*)((uint)str + (uint)i))); 
      memmove((void*)keys[tagCount].key, (void*)(char*)((uint)str + i), (uint)strlen((char*)(uint)str+i)+1); 
      cprintf("copied key is: %s at keys[%d]\n", keys[tagCount].key, tagCount);
      tagCount++;
    }
  }
  return tagCount;
}
/*
int
getFilesByTag_back(struct file* f, char* key, char* value, int valueLength, char* results, int resultsLength)
{
  int j = 0;
  int k = 0;
  struct buf *bp;
  uchar str[BSIZE];
  int keyPos = 0;
  int file_value_len = 0;
  char *file_value;
  struct dirent *d;
  char *filename;
  int filenameLength = 0;
  //uint off;
  //struct buf *bp_dir;
  
  if(!f->ip->tags) {
    cprintf("tags not even there");
    return 0;
  }
  bp = bread(f->ip->dev, f->ip->tags);
  memmove((void*)str, (void*)bp->data, (uint)BSIZE);
  brelse(bp);
  if((keyPos = searchKey((uchar*)key, (uchar*)str)) >= 0) {
    file_value_len = 17;
    file_value = (char*)((uint)str + (uint)keyPos + 10);
    while (file_value_len >= 0 && !file_value[file_value_len]) file_value_len--;
    file_value_len++;
    if(file_value_len == valueLength) {
    for(j = 0; j < valueLength && file_value[j] == value[j]; j++);
      if(j == valueLength) {
        //
        cprintf("1\n");
        ilock(f->ip);
        cprintf("2\n");
        for(off = 0; off < f->ip->size; off += BSIZE){
          bp_dir = bread(f->ip->dev, bmap(f->ip, off / BSIZE));
          for(d = (struct dirent*)bp_dir->data;
              d < (struct dirent*)(bp_dir->data + BSIZE);
              d++){
            if(d->inum == f->ip->inum) {
              cprintf("found name\n");
              filename = d->name;
              break;
            }
         }
       }
       brelse(bp_dir);
        iunlock(f->ip);
        if(filename){
          d = (struct dirent*)str;
          if (d->inum) {
            k = resultsLength - 1;
             // go till non-empty
            while (k >= 0 && !results[k]) k--;
            // actual length
            k++;
            // go till empty
            if (k) k++;
            filename = d->name;
            filenameLength = strlen(filename);
            cprintf("file name is: %s\n", filename);
           // enough room
          if(resultsLength - k >= filenameLength) {
            memmove((void*)((uint)results + (uint)k), (void*)filename, (uint)filenameLength);
            cprintf("copied and is: %s\n", (char*)((uint)results + (uint)k)); 
            results[filenameLength] = NULL;
            return 1;
          } else {
              // not enough room
            cprintf("not enough room\n");
            return -1;
          }
        }
      }
    }
  }
  return -1;
}
*/
int
getFilesByTag(char* key, char* value, int valueLength, char* results, int resultsLength)
{
  struct inode *root_start;
  struct inode *each_inode;
  struct buf *bp;
  struct buf *tagbf;
  struct dirent *de;
  uchar str[BSIZE];
  uint off;
  int file_value_len = 0;
  int file_name_len;
  int j, tag_pos, k;
  int count = 0;
  char *file_value;
  char *file_name;
  // get the root directory -from namex
  root_start = iget(ROOTDEV, ROOTINO);
  // lock inode
  for(off = 0; off < root_start->size; off += BSIZE) {
    bp = bread(root_start->dev, bmap(root_start, off / BSIZE));
    for(de = (struct dirent*)bp->data;
        de < (struct dirent*)(bp->data + BSIZE);
        de++){
      if(de->inum == 0)
        continue;
      each_inode = iget(root_start->dev, de->inum);
      if(!each_inode->tags)
        continue;
      tagbf = bread(root_start->dev, each_inode->tags);
      memmove((void*)str, (void*)tagbf->data, (uint)BSIZE);
      brelse(tagbf);
      tag_pos = searchKey((uchar*)key, (uchar*)str);
      if(tag_pos >= 0) {
        file_value_len = 17;
        file_value = (char*)((uint)str + (uint)tag_pos + 10);
        while (file_value_len >= 0 && !file_value[file_value_len]) file_value_len--;
        file_value_len++;
        if(file_value_len == valueLength) {
          for(j = 0; j < valueLength && file_value[j] == value[j]; j++);
          if(j == valueLength) {
            count++;
            file_name = de->name;
            cprintf("found file name is: %s\n\n", file_name);
            if(file_name){
              k = resultsLength - 1;
              while (k >= 0 && !results[k]) k--;
              // actual length
              k++;
              // go till empty
              if (k) k++;
              file_name_len = strlen(file_name);
              cprintf("file name length is: %d\n", file_name_len);
              // enough room
              if(resultsLength - k >= file_name_len) {
                memmove((void*)((uint)results + (uint)k), (void*)file_name, (uint)file_name_len + 1);
                cprintf("copied and is: %s\n", (char*)((uint)results + (uint)k)); 
                results[k+file_name_len] = NULL;
              } else {
              // not enough room
                cprintf("not enough room");
                brelse(bp);
                return -1;
              }
            }
          }
        }
      } else {
        continue;
      }
    }
    brelse(bp);
  }
  return count;
}
