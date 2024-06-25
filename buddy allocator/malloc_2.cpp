#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define MAX_SIZE 100000000

void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

struct MallocMetadata{
   size_t requested_size;
   bool is_free;
   MallocMetadata* next;
   MallocMetadata* prev;
};

MallocMetadata* gobal_pointer=nullptr;

void* smalloc(size_t size){
    if(size==0||size> MAX_SIZE)return NULL;
    MallocMetadata* temp =gobal_pointer;
  //  bool is_found=false;
    MallocMetadata* prev=nullptr;
    while (temp!=nullptr){
        if(temp->requested_size>=size&& temp->is_free==true)
        {
            temp->is_free=false;
            return (void*)((char*)temp + sizeof(MallocMetadata));

        }
        prev=temp;
        temp=temp->next;
    }

    void* newblock = sbrk(size + sizeof(MallocMetadata));

    if(newblock == (void*)-1) return NULL;

    MallocMetadata* new_allocated = (MallocMetadata*) newblock;
    new_allocated->prev = prev;
  
    new_allocated->next = NULL;
    new_allocated->requested_size = size;
    new_allocated->is_free = false;

     if (prev!=nullptr)
    {
        prev->next=new_allocated;
    }
    if (gobal_pointer==nullptr)
    {
        gobal_pointer= new_allocated;
    }

    return (char*)newblock + sizeof(MallocMetadata);
}

void* scalloc(size_t num, size_t size){

    if (num == 0 || size == 0 || num * size > MAX_SIZE)  return NULL;
    void* new_block= smalloc(num*size);
    if(new_block==NULL) return NULL;

    memset(new_block, 0, num*size);
    return new_block;
}

void sfree(void* p){
    if(p==NULL)return;
    char* p_char = static_cast<char*>(p);
    MallocMetadata* temp = static_cast<MallocMetadata*>(static_cast<void*>(p_char - sizeof(MallocMetadata)));
    temp->is_free=true;
}

void* srealloc(void* oldp, size_t size){
    if(size> MAX_SIZE || size == 0) return NULL;
    if (oldp == NULL) {
        return smalloc(size);
    }
    char* p_char = static_cast<char*>(oldp);
    MallocMetadata* temp = static_cast<MallocMetadata*>(static_cast<void*>(p_char - sizeof(MallocMetadata)));
    if(temp->requested_size>= size) return oldp;

    void* res = smalloc(size);
    if (res==NULL)return NULL;

    memmove(res, oldp, temp->requested_size);
    sfree(oldp);
    return res;
}

size_t _num_free_blocks() {
    size_t counter=0;
    MallocMetadata* temp= gobal_pointer;
    while(temp!=nullptr)
    {
        counter += (temp->is_free);
        temp= temp->next;
    }
    return counter;
}

size_t _num_free_bytes(){
    size_t counter=0;
    MallocMetadata* temp= gobal_pointer;
    while(temp!=nullptr)
    {
        if(temp->is_free)counter+= temp->requested_size;

        temp= temp->next;
    }
    return counter;
}


size_t _num_allocated_blocks(){
     size_t counter=0;
    MallocMetadata* temp= gobal_pointer;
    while(temp!=nullptr)
    {
        counter++;
        temp= temp->next;
    }
    return counter;
}


size_t _num_allocated_bytes(){
    size_t counter=0;
    MallocMetadata* temp= gobal_pointer;
    while(temp!=nullptr)
    {
        counter+= temp->requested_size;
        
        temp= temp->next;
    }
    return counter;
}


size_t _num_meta_data_bytes() {
    return _num_allocated_blocks() * sizeof(MallocMetadata);
}

size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}





