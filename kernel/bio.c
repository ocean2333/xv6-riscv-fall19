// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13

struct {
  struct spinlock lock[NBUCKETS];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  //struct buf head;
  struct buf hashbucket[NBUCKETS];
} bcache;

int
hash(struct buf *b){
  return b->blockno % 13;
}

void
binit(void)
{
  struct buf *b;
  for(int i=0;i<NBUCKETS;i++){
    initlock(&(bcache.lock[i]), "bcache");
    // Create linked list of buffers
    bcache.hashbucket[i].prev = &(bcache.hashbucket[i]);
    bcache.hashbucket[i].next = &(bcache.hashbucket[i]);
  }
  
  // Create linked list of buffers
  //bcache.head.prev = &bcache.head;
  //bcache.head.next = &bcache.head;
  int d = 0;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    int c = d++ % 13;
    b->next = bcache.hashbucket[c].next;
    b->prev = &bcache.hashbucket[c];
    initsleeplock(&b->lock, "buffer");
    bcache.hashbucket[c].next->prev = b;
    bcache.hashbucket[c].next = b;
    

  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int c = blockno % 13;
  acquire(&(bcache.lock[c]));

  // Is the block already cached?
  for(b = bcache.hashbucket[c].next; b != &bcache.hashbucket[c]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&(bcache.lock[c]));
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; recycle an unused buffer.
  for(b = bcache.hashbucket[c].prev; b != &bcache.hashbucket[c]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&(bcache.lock[c]));
      acquiresleep(&b->lock);
      return b;
    }
  }
  
  //use other bucket's cache
  for(int i=0;i<NBUCKETS;i++){
    if(i==c) continue;
    acquire(&(bcache.lock[i]));
    for(b = bcache.hashbucket[i].prev; b != &bcache.hashbucket[i]; b = b->prev){
      if(b->refcnt == 0) {

        b->prev->next = b->next;
        b->next->prev = b->prev;
        b->prev = &bcache.hashbucket[c];
        b->next = bcache.hashbucket[c].next;
        bcache.hashbucket[c].next->prev = b;
        bcache.hashbucket[c].next = b;

        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        release(&(bcache.lock[c]));
        acquiresleep(&b->lock);
        release(&(bcache.lock[i]));       
        return b;
      }
    }
    release(&(bcache.lock[i]));
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int c = hash(b);

  acquire(&bcache.lock[c]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.hashbucket[c].next;
    b->prev = &bcache.hashbucket[c];
    bcache.hashbucket[c].next->prev = b;
    bcache.hashbucket[c].next = b;
  }
  
  release(&bcache.lock[c]);
}

void
bpin(struct buf *b) {
  int c = hash(b);
  acquire(&bcache.lock[c]);
  b->refcnt++;
  release(&bcache.lock[c]);
}

void
bunpin(struct buf *b) {
  int c = hash(b);
  acquire(&bcache.lock[c]);
  b->refcnt--;
  release(&bcache.lock[c]);
}

