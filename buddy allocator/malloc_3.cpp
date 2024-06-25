#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include<iostream>
#define MAX_SIZE 100000000
#define BLOCK_SIZE (1024*128)
#define MAX_ORDER 10
#define MIN_BLOCK 128
#define INITIAL_BLOCK 32

size_t allocated_block=0;
size_t allocated_bytes=0;
size_t free_bytes=0;
size_t free_block =0;
size_t metaDataBlocks = 0;
bool is_initialized =false;
unsigned long globalPointer=0;
void combine(void *p,size_t m_maxCapSize);
void* getFreeBlock(size_t size);
void deleteMappedBl(void *p);
void removeblock(void* p);
bool isMergible(void *p);
unsigned long gtbudd(void* p, size_t custSiz);
void putNew(size_t size,void* ptr);
int findOrder(size_t size);
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
void* mallocLastPart(void * block,size_t size);

int findOrder(size_t size){
    int counter = 0;
    size_t right = MIN_BLOCK;
    size_t left = 0;
    while (counter <= MAX_ORDER) {
        if (size>right||size<left) {
            counter++;
            left = right;
            right *= 2;
        }
        else{
            return counter;
        }
    }
    return counter;
}
struct MallocMetaData
{
   size_t requested_size;
   bool is_free;
   MallocMetaData* next;
   MallocMetaData* prev;
};
    MallocMetaData* attemptMrg(size_t targetSize,MallocMetaData* p);
    MallocMetaData* free_blocks_array[MAX_ORDER+1];
    MallocMetaData* maped_list =nullptr;
  



void addBFree(void* p){
    if(p==nullptr)return;
     MallocMetaData* temp= (MallocMetaData*)p;
     int find_order= findOrder(temp->requested_size+sizeof(MallocMetaData));
     if (find_order==-1)return;
     else{

        temp->is_free=true;
        if(free_blocks_array[find_order]!=nullptr)
        {
            MallocMetaData* helper= free_blocks_array[find_order];
            while ((unsigned long)p > (unsigned long)helper)
            {
                if(helper->next==nullptr)break;
                helper=helper->next;
            }
            if(helper->next==nullptr)
            {
                temp->prev= helper;
                temp->next=nullptr;
                helper->next=temp;
                return;  
            }

            temp->prev=helper->prev;
            temp->next= helper;
            helper->prev= temp;
            if(temp->prev==nullptr) 
            {
               free_blocks_array[find_order]=temp;
            }
            else
            {
                MallocMetaData* prev_up = temp->prev;
                prev_up->next=temp;
            }
              

            
        }
        else{
            free_blocks_array[find_order]=temp;
        }

     }
}




void removeblock(void* p){
   if(p==nullptr)return;
   MallocMetaData* temp= (MallocMetaData*)p;
   if(!temp->is_free)return;
   int find_order= findOrder(temp->requested_size);
   if (find_order==-1)return;
   else
   {
        if(free_blocks_array[find_order]!=temp){
            if(temp->prev==nullptr&&temp->next==nullptr)return;
            MallocMetaData* prev = temp->prev;
            MallocMetaData* next = temp->next;
            prev->next= next;
            if(next!=nullptr)
            {
               next->prev=prev;
            }
            
        }
        else
        {
            free_blocks_array[find_order]= temp->next;
            if(temp->next!=nullptr) temp->next->prev=nullptr;
        }
        
   }
   temp->prev=temp->next=nullptr;
   
 
}



void putNew(size_t size,void* ptr){
    int order = findOrder(size);
    MallocMetaData* m_cur = (MallocMetaData*) ptr;
    removeblock(m_cur);
    int currentOrder = findOrder(sizeof(MallocMetaData)+m_cur->requested_size);
    size_t currentSize = sizeof(MallocMetaData)+m_cur->requested_size;
    while(currentOrder > order){
        currentOrder--;
        currentSize=(currentSize)/2;
        MallocMetaData* buddy = (MallocMetaData*)((char*)m_cur+currentSize);
        buddy->is_free=true;
        buddy->requested_size=currentSize - sizeof(MallocMetaData);
        addBFree(buddy);
        m_cur->requested_size=currentSize - sizeof(MallocMetaData);
        metaDataBlocks++;
        free_block++;
        allocated_block++;
        free_bytes-=sizeof(MallocMetaData);
        allocated_bytes-=sizeof(MallocMetaData);
    }
    
}




void* smalloc(size_t size){
    if (!is_initialized) {
        free_block =INITIAL_BLOCK;
        allocated_block=INITIAL_BLOCK;
        metaDataBlocks=INITIAL_BLOCK;
        allocated_bytes =  BLOCK_SIZE*INITIAL_BLOCK - sizeof(MallocMetaData)*INITIAL_BLOCK;
        free_bytes = BLOCK_SIZE*INITIAL_BLOCK - sizeof(MallocMetaData)*INITIAL_BLOCK;
        is_initialized = true;
        size_t additionalMemory = (INITIAL_BLOCK*BLOCK_SIZE-(unsigned long)sbrk(0))%(INITIAL_BLOCK*BLOCK_SIZE);
        if(sbrk(additionalMemory)== nullptr){
             return nullptr;
         }
        void* res =sbrk(INITIAL_BLOCK*BLOCK_SIZE) ;
        if(res == nullptr){
            return nullptr;
        }
        globalPointer = (unsigned long)res;
        char* currentAddress =(char*) globalPointer;
        for(int i=0;i<INITIAL_BLOCK;++i){
            MallocMetaData* metaDataPtr = (MallocMetaData*)currentAddress;
            MallocMetaData* prev = (i != 0) ?  (MallocMetaData*)((char*)currentAddress-BLOCK_SIZE):nullptr;
            MallocMetaData* next = (i != INITIAL_BLOCK-1) ? (MallocMetaData*)(BLOCK_SIZE+(char*)currentAddress):nullptr;
            metaDataPtr->prev = prev;
            metaDataPtr->next = next;
            metaDataPtr->is_free = true;
            metaDataPtr->requested_size = BLOCK_SIZE-sizeof(MallocMetaData);
            currentAddress+=BLOCK_SIZE;
        }
        free_blocks_array[MAX_ORDER] = (MallocMetaData*)globalPointer;
        for(int i=0;i<MAX_ORDER;++i){ 
            free_blocks_array[i]= nullptr;
        }
        
    }
    if(size > MAX_SIZE){
        return nullptr;
    }
    size_t tot_size = sizeof(MallocMetaData) + size;
    if (tot_size > BLOCK_SIZE) {
        void* temp = (void*)mmap(nullptr,sizeof(MallocMetaData)+size,PROT_READ | PROT_WRITE,MAP_ANONYMOUS | MAP_PRIVATE,-1,0);
        if(temp == nullptr)return NULL;
        MallocMetaData* meta_data= (MallocMetaData*)temp;
        meta_data->requested_size= size;
        meta_data->is_free=false;
        metaDataBlocks=metaDataBlocks+1;
        allocated_bytes+=size;
        allocated_block++;
        if (maped_list!= nullptr)
        {
            maped_list->prev= meta_data;
            meta_data->next=maped_list;
            meta_data->prev=nullptr;
            maped_list=meta_data;
        }
        else
        {
            maped_list= meta_data;
        }

        return (void*)((char*)meta_data+sizeof(MallocMetaData));
    }
    void* block = getFreeBlock(tot_size);
    return mallocLastPart(block,tot_size);
   
}


void* scalloc(size_t num, size_t size){
    if (num == 0 || size == 0 || num * size > MAX_SIZE)  
        return nullptr;
    void* new_block= smalloc(num*size);
    if(new_block==nullptr)    
        return nullptr;
    memset(new_block, 0, num*size);
    return new_block;
}

void sfree(void* p){
    if(p==nullptr)
        return ;
    MallocMetaData* m_metaData=(MallocMetaData*)((char*)p-sizeof(MallocMetaData));
    if(m_metaData->is_free)
        return;////////already freed
    if(m_metaData->requested_size<=BLOCK_SIZE){
        m_metaData->is_free=true;
        combine(m_metaData,0);
    }
    else{
        deleteMappedBl(m_metaData);
        m_metaData->is_free=true;
        munmap((void*)(m_metaData),m_metaData->requested_size+sizeof(*m_metaData));
    }
}

size_t _num_free_blocks(){
    return free_block;
}

size_t _num_free_bytes(){
    return free_bytes;
}

size_t _num_allocated_blocks(){
    return allocated_block;
}

size_t _num_allocated_bytes(){
    return allocated_bytes;
}
size_t _num_meta_data_bytes(){
    return sizeof(MallocMetaData)*metaDataBlocks;
}
size_t _size_meta_data(){
    return sizeof(MallocMetaData);
}





void combine(void *p,size_t m_maxCapSize){
    MallocMetaData* m_merged;
    MallocMetaData* m_bud;
    free_block=free_block+1;
    MallocMetaData * m_meta=(MallocMetaData*)p;
    free_bytes=free_bytes+m_meta->requested_size;
    while(isMergible(m_meta)){
        if (m_maxCapSize>0 &&m_meta->requested_size>=m_maxCapSize){
            break;
        }
        m_bud=(MallocMetaData*)gtbudd(m_meta,0);
        m_merged= (MallocMetaData*)((unsigned long)m_bud > (unsigned long)m_meta) ? m_meta : m_bud;
        size_t bigSiz=2*(sizeof(*m_meta)+m_bud->requested_size)-sizeof(MallocMetaData);//*m_meta  instead of MallocMetaData struct.
        removeblock(m_bud);
        removeblock(m_meta);
        free_block=free_block-1;
        m_merged->is_free=true;
        m_merged->requested_size=bigSiz;
        m_meta=m_merged;
        allocated_block=allocated_block-1;
        metaDataBlocks=metaDataBlocks-1;
        free_bytes=sizeof(MallocMetaData)+free_bytes;
        allocated_bytes=sizeof(MallocMetaData)+allocated_bytes;
    }
    addBFree(m_meta);
}



void* srealloc(void* oldp, size_t size){
    if(size>= MAX_SIZE || size == 0) 
        return nullptr;
    if (oldp == nullptr) {
        return smalloc(size);
    }
    MallocMetaData* m_meta =(MallocMetaData*)((char*)oldp-sizeof(MallocMetaData));
    if (BLOCK_SIZE<size)
    {
        if (m_meta->requested_size!=size){
            void* mshndlHugeBlock = smalloc(size);
            memmove(mshndlHugeBlock,oldp,size);
            sfree(oldp);
            return mshndlHugeBlock;
        }
        return oldp;
    }
    if(m_meta->requested_size>=size){
        return oldp;
    }
    void * res=nullptr;
    MallocMetaData* m_mrg=attemptMrg(size,m_meta);
    if(m_mrg==nullptr){
        res=smalloc(size);
        if(res==nullptr)
            return nullptr;
        MallocMetaData* temp=(MallocMetaData*)((char*)oldp-sizeof(MallocMetaData));
        memmove(res,oldp,temp->requested_size);
        sfree(oldp);
        return res;
    }
    res=(void*)((char*)m_mrg+sizeof(MallocMetaData));
    memmove(res,oldp,size);
    free_bytes=free_bytes-m_mrg->requested_size;
    free_block=free_block-1;
    m_mrg->is_free=false;
    return res;
}



MallocMetaData* attemptMrg(size_t targetSize,MallocMetaData* p){
    if(p==nullptr)
        return nullptr;
    MallocMetaData* m_bud = nullptr;
    size_t m_tot = sizeof(*m_bud)+targetSize ;
    MallocMetaData* m_meta = p;
    size_t m_currLength=sizeof(*m_bud)+m_meta->requested_size;
    while (m_currLength<BLOCK_SIZE&&m_tot >m_currLength){
        m_bud=(MallocMetaData*)gtbudd(m_meta,m_currLength-sizeof(MallocMetaData));
        if(m_bud->is_free==false){
            break;
        }
        m_meta=(MallocMetaData*)((unsigned long)m_bud <= (unsigned long)m_meta) ? m_bud : m_meta;
        m_currLength=2*m_currLength;

    }

    if(m_currLength>=m_tot&&m_currLength<=BLOCK_SIZE){
        combine(p,m_currLength-sizeof(*m_meta));
        return m_meta;
    }
    return nullptr;
}



bool isMergible(void *p){
    MallocMetaData* m_bud;
    MallocMetaData* m_meta;
    if(!p)
        return false;
    m_meta=(MallocMetaData*)p;
    if(BLOCK_SIZE <=sizeof(*m_meta)+m_meta->requested_size)
        return false;
    m_bud=(MallocMetaData*)gtbudd(p,0);
    if(m_bud->is_free==false)
        return false;
    if(m_meta->requested_size==m_bud->requested_size)
        return true;
    return false;
}


unsigned long gtbudd(void* p, size_t custSiz){
    MallocMetaData* m_meta = (MallocMetaData*)p;
    size_t length;
    if(custSiz!=0)
        length=custSiz;
    else 
        length=m_meta->requested_size;
    return ((unsigned long)(sizeof(*m_meta)+length) ^ (unsigned long)m_meta);
}


void *mallocLastPart(void * block,size_t size){
     if(block == nullptr){
        return nullptr;
    }
    putNew(size,block);
    MallocMetaData* m_met = (MallocMetaData*) block;
    free_bytes-=m_met->requested_size;
    free_block--;
    m_met->is_free=false;
    return (void*)((char*)m_met+sizeof(MallocMetaData));
}
void deleteMappedBl(void *p){
    ////////////jahez
    if(p == nullptr){
        return;
    }
    metaDataBlocks=metaDataBlocks-1;
    MallocMetaData* m_meta = (MallocMetaData*) p;
    allocated_bytes=allocated_bytes-m_meta->requested_size;
    MallocMetaData* m_next = m_meta->next;
    allocated_block=allocated_block-1;
    MallocMetaData* m_pre = m_meta->prev;
    m_meta->is_free=true;
    if (maped_list!=p){
        if (m_next!=nullptr)
        {
            m_pre->next=m_next;
            m_next->prev=m_pre;
        }
        else{
            m_pre->next=nullptr;
        }
        
    }
    else{
        if(m_next!=nullptr){
            maped_list=m_next;
            m_next->prev=nullptr;
        }
        else{
            maped_list=nullptr;
            return;
        }
    }
    m_meta->prev=m_meta->next=nullptr;
}




void* getFreeBlock(size_t size){
    int m_ord = findOrder(size);
    if(m_ord == -1){
        return nullptr; 
    }
    while(m_ord <= MAX_ORDER){
        if(free_blocks_array[m_ord]!=nullptr){
            return free_blocks_array[m_ord] ;
        }
        m_ord++;
    }
    return nullptr;

}

