// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

//changed code to use array inplace of list - OS Assignment 5 (112103099)

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#define MAX_ARR_SIZE (PHYSTOP/PGSIZE) 

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run 
{
  struct run *next;
  //array was added here
  char* free_arr [MAX_ARR_SIZE];   
  int free_arr_size;

};

struct 
{
  struct spinlock lock;
  int use_lock;
  struct run freeArr;
} kmem;


// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  kmem.freeArr.free_arr_size = 0;
  freerange(vstart, vend);
}

void kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)

void kfree(char *v)
{
  //struct run *r;    //will use array instead

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
  {
    acquire(&kmem.lock);
  }
  //r = (struct run*)v;
  //r->next = kmem.freelist;
  //kmem.freelist = r;
  
  //check if array size is less than the max size possible
  if (kmem.freeArr.free_arr_size < MAX_ARR_SIZE) 
  {   
	  kmem.freeArr.free_arr[kmem.freeArr.free_arr_size] = v;    //add it to the free array
	  kmem.freeArr.free_arr_size++;
  }
  else 
  {
       panic("Array size exceeded\n");
  }

  //keep this code as it is
  if(kmem.use_lock)
  {
    release(&kmem.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.

char* kalloc(void)
{
  //struct run *r;
  char* allocateNewPage = 0;

  if(kmem.use_lock)
  {
	  acquire(&kmem.lock);
  }
  //r = kmem.freelist;
  //if(r)
    //kmem.freelist = r->next;

  int currentIndex = kmem.freeArr.free_arr_size;

  if(currentIndex > 0)
  {
	  currentIndex--;
	  allocateNewPage = kmem.freeArr.free_arr[currentIndex];
	  kmem.freeArr.free_arr_size = currentIndex;
  }
  else
  {
	  panic("Index out of lower bound\n");
  }

  if(kmem.use_lock)
  {
    release(&kmem.lock);
  }
  //return (char*)r;
  return allocateNewPage;
}

