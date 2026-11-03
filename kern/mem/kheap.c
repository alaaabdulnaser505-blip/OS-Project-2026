#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include <kern/conc/sleeplock.h>
#include <kern/proc/user_environment.h>
#include <kern/mem/memory_manager.h>
#include "../conc/kspinlock.h"
#include "../conc/ksemaphore.h"
#include <kern/cpu/cpu.h>

#define PERM_BUDDY 0x800

static uint32 largeAllocPages[NUM_OF_KHEAP_PAGES];

struct KheapFreeArea
{
    uint32 start;
    uint32 pages;
};

static struct KheapFreeArea freeAreas[NUM_OF_KHEAP_PAGES];
static uint32 freeAreaCount = 0;

static struct kspinlock kheap_lock;

/************************************************************************************/
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//TODO: [PROJECT'26.GM#2] KERNEL HEAP - [0] GIVEN FUNCTIONS
//==============================================
// [1] INITIALIZE KERNEL HEAP:
//==============================================
void kheap_init()
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		buddy_init();
		set_kheap_strategy(KHP_PLACE_CUSTOMFIT);
		kheapPageAllocStart = KERNEL_HEAP_START;
		kheapPageAllocBreak = KERNEL_HEAP_START;
        init_kspinlock(&kheap_lock, "kheap");
        freeAreaCount = 0;
	}
	//==================================================================================
	//==================================================================================
}
/************************************************************************************/

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
/**************************************************/
/* BLOCK ALLOCATOR */
/**************************************************/

void* create_buddy_space()
{
	//TODO: [PROJECT'26.GM#2] KERNEL HEAP - [4] create_buddy_space
	//Your code is here
	 uint32 pageVirtualAddress = kheapPageAllocBreak;

    if (pageVirtualAddress > KERNEL_HEAP_MAX ||
        PAGE_SIZE > KERNEL_HEAP_MAX - pageVirtualAddress)
    {
        return NULL;
    }

    if (alloc_page(ptr_page_directory,
                   pageVirtualAddress,
                   PERM_WRITEABLE | PERM_BUDDY,
                   1) != 0)
    {
        return NULL;
    }

    kheapPageAllocBreak = pageVirtualAddress + PAGE_SIZE;

    return (void*)pageVirtualAddress;
	//Comment the following line
	//panic("create_buddy_space() not implemented");
}
static void shrink_kheap_break()
{
    while (kheapPageAllocBreak > KERNEL_HEAP_START)
    {
        // Check whether the last free area reaches the break
        if (freeAreaCount > 0)
        {
            uint32 last = freeAreaCount - 1;

            uint32 areaEnd = freeAreas[last].start +
                             freeAreas[last].pages * PAGE_SIZE;

            if (areaEnd == kheapPageAllocBreak)
            {
                kheapPageAllocBreak = freeAreas[last].start;
                remove_free_area(last);
                continue;
            }
        }

        // Check whether the last page is already unmapped
        uint32 lastPage = kheapPageAllocBreak - PAGE_SIZE;
        uint32 *page_table = NULL;

        struct FrameInfo *frame = get_frame_info(
            ptr_page_directory, lastPage, &page_table);

        // This can happen when a Buddy page is released
        if (frame == NULL)
        {
            kheapPageAllocBreak -= PAGE_SIZE;
            continue;
        }

        // The last page is still allocated
        break;
    }
}
void  delete_buddy_space(void *va)
{
	//TODO: [PROJECT'26.GM#2] KERNEL HEAP - [5] delete_buddy_space
	//Your code is here
	//Comment the following line
	//panic("delete_buddy_space() not implemented");
	
    if (va == NULL)
    	panic("NULL address");
	uint32 buddyPageVA = ROUNDDOWN((uint32)va, PAGE_SIZE);

    int buddyPagePerms = pt_get_page_permissions(ptr_page_directory, buddyPageVA);

   if (buddyPageVA < kheapPageAllocStart ||
		buddyPageVA >= kheapPageAllocBreak ||
		buddyPagePerms < 0 || !(buddyPagePerms & PERM_PRESENT) || !(buddyPagePerms & PERM_BUDDY))
    {
        panic("invalid buddy space");
    }
	
   pt_set_page_permissions(ptr_page_directory, buddyPageVA, 0, PERM_BUDDY);
	
   unmap_frame(ptr_page_directory, buddyPageVA);
   add_free_area(buddyPageVA, 1);
   shrink_kheap_break();
}


/**************************************************/
/* PAGE ALLOCATOR */
/**************************************************/
static void remove_free_area(uint32 index)
{
    for (uint32 i = index; i + 1 < freeAreaCount; i++)
        freeAreas[i] = freeAreas[i + 1];

    freeAreaCount--;
}


static void add_free_area(uint32 start, uint32 pages)
{
    if (pages == 0)
        return;

    if (freeAreaCount >= NUM_OF_KHEAP_PAGES)
        panic("No space for free area metadata");

    uint32 i = 0;

    // Keep free areas sorted by virtual address.
    while (i < freeAreaCount && freeAreas[i].start < start)
        i++;

    for (uint32 j = freeAreaCount; j > i; j--)
        freeAreas[j] = freeAreas[j - 1];

    freeAreas[i].start = start;
    freeAreas[i].pages = pages;
    freeAreaCount++;

    // Merge with previous area if adjacent.
    if (i > 0)
    {
        uint32 previousEnd =
            freeAreas[i - 1].start + freeAreas[i - 1].pages * PAGE_SIZE;

        if (previousEnd == freeAreas[i].start)
        {
            freeAreas[i - 1].pages += freeAreas[i].pages;
            remove_free_area(i);
            i--;
        }
    }

    // Merge with next area if adjacent.
    if (i + 1 < freeAreaCount)
    {
        uint32 currentEnd =
            freeAreas[i].start + freeAreas[i].pages * PAGE_SIZE;

        if (currentEnd == freeAreas[i + 1].start)
        {
            freeAreas[i].pages += freeAreas[i + 1].pages;
            remove_free_area(i + 1);
        }
    }
}



void* kmalloc(unsigned int size)
{
#if USE_KHEAP

    if (size == 0)
        return NULL;

    if (size <= DYN_ALLOC_MAX_BLOCK_SIZE - META_DATA_SIZE)
    {
        acquire_kspinlock(&kheap_lock);
        void* block = alloc_block(size);
        release_kspinlock(&kheap_lock);

        return block;
    }

    uint32 no_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;

    acquire_kspinlock(&kheap_lock);

    int selected_area = -1;

    // 1. Search for Exact Fit
    for (uint32 i = 0; i < freeAreaCount; i++)
    {
        if (freeAreas[i].pages == no_pages)
        {
            selected_area = i;
            break;
        }
    }

    // 2. If Exact Fit was not found, search for Worst Fit
    if (selected_area == -1)
    {
        uint32 largest_area = 0;

        for (uint32 i = 0; i < freeAreaCount; i++)
        {
            if (freeAreas[i].pages >= no_pages &&
                freeAreas[i].pages > largest_area)
            {
                largest_area = freeAreas[i].pages;
                selected_area = i;
            }
        }
    }

    uint32 start_address;

    // 3. Use a free area if one was found
    if (selected_area != -1)
    {
        start_address = freeAreas[selected_area].start;
    }
    else
    {
        // 4. Otherwise, extend the heap break
        uint32 available_pages =
            (KERNEL_HEAP_MAX - kheapPageAllocBreak) / PAGE_SIZE;

        if (no_pages > available_pages)
        {
            release_kspinlock(&kheap_lock);
            return NULL;
        }

        start_address = kheapPageAllocBreak;
    }

    // Allocate the required pages
    uint32 mapped_pages = 0;

    for (; mapped_pages < no_pages; mapped_pages++)
    {
        uint32 address = start_address + mapped_pages * PAGE_SIZE;

        if (alloc_page(ptr_page_directory,
                       address,
                       PERM_WRITEABLE,
                       1) != 0)
        {
            // Roll back pages allocated before the failure
            while (mapped_pages > 0)
            {
                mapped_pages--;
                unmap_frame(ptr_page_directory,
                            start_address + mapped_pages * PAGE_SIZE);
            }

            release_kspinlock(&kheap_lock);
            return NULL;
        }
    }

    // Record the allocation size at its starting page
    uint32 page_index =
        (start_address - KERNEL_HEAP_START) / PAGE_SIZE;

    largeAllocPages[page_index] = no_pages;

    // Update the selected free area
    if (selected_area != -1)
    {
        freeAreas[selected_area].start += no_pages * PAGE_SIZE;
        freeAreas[selected_area].pages -= no_pages;

        if (freeAreas[selected_area].pages == 0)
            remove_free_area(selected_area);
    }
    else
    {
        kheapPageAllocBreak += no_pages * PAGE_SIZE;
    }

    release_kspinlock(&kheap_lock);

    return (void*)start_address;

#else
    panic("Kernel Heap is disabled");
    return NULL;
#endif
}

//================================
//[2] free space from kernel heap:
//================================
void kfree(void* virtual_address)
{
#if USE_KHEAP

    if (virtual_address == NULL)
        return;

    uint32 va = (uint32)virtual_address;

    if (va < KERNEL_HEAP_START || va >= KERNEL_HEAP_MAX)
        panic("kfree: Invalid kernel heap address");

    uint32 *page_table = NULL;

    struct FrameInfo *frame = get_frame_info(
        ptr_page_directory, va, &page_table);

    if (frame == NULL)
        panic("kfree: Address is not allocated");

    uint32 page_entry = page_table[PTX(va)];

    // Small allocation: Buddy System
    if (page_entry & PERM_BUDDY)
    {
        acquire_kspinlock(&kheap_lock);
        free_block(virtual_address);
        release_kspinlock(&kheap_lock);
        return;
    }

    // Large allocation must start at a page boundary
    if (va % PAGE_SIZE != 0)
        panic("kfree: Invalid allocation address");

    uint32 page_index =
        (va - KERNEL_HEAP_START) / PAGE_SIZE;

    uint32 no_pages = largeAllocPages[page_index];

    if (no_pages == 0)
        panic("kfree: Invalid allocation address");

    acquire_kspinlock(&kheap_lock);

    // Free all pages belonging to this allocation
    for (uint32 i = 0; i < no_pages; i++)
    {
        unmap_frame(ptr_page_directory,
                    va + i * PAGE_SIZE);
    }

    // Clear allocation information
    largeAllocPages[page_index] = 0;

    // Add the freed area and merge adjacent free areas
add_free_area(va, no_pages);

shrink_kheap_break();

release_kspinlock(&kheap_lock);
#else

    panic("Kernel Heap is disabled");

#endif
}

//================================
//[3] find PA of given VA:
//================================

unsigned int kheap_physical_address(unsigned int virtual_address)
{
#if USE_KHEAP
    if (virtual_address < KERNEL_HEAP_START ||
        virtual_address >= kheapPageAllocBreak)
        return 0;

    uint32 *page_table = NULL;

    struct FrameInfo *frame_info = get_frame_info(
        ptr_page_directory,
        virtual_address,
        &page_table
    );

    if (frame_info == NULL)
        return 0;

    return to_physical_address(frame_info) +
           PGOFF(virtual_address);
#else
    panic("Kernel Heap is disabled");
    return 0;
#endif
}



//================================
//[4] find VA of given PA:
//================================


unsigned int kheap_virtual_address(unsigned int physical_address)
{
#if USE_KHEAP

    if (physical_address == 0)
        return 0;

    if (PPN(physical_address) >= number_of_frames)
        return 0;

    struct FrameInfo *frame_info =
        to_frame_info(physical_address);

    if (frame_info->kheap_va == 0)
        return 0;

    return frame_info->kheap_va + PGOFF(physical_address);

#else

    panic("Kernel Heap is disabled");
    return 0;

#endif
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().
void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'26.BONUS#1] KERNEL REALLOC - krealloc
	//Your code is here
	//Comment the following line
	panic("krealloc() is not implemented yet...!!");
}
