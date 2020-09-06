/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines.
 */

#include "mm_alloc.h"
#include <stdlib.h>


void mm_print_mem() {

    printf( "\nstart_addr\tsize\tfree\tnext\n");
    printf("=============================\n");

    // check if first block is free

    printf("HEAD OF LL %ld\n", head);
   struct header_t *curr = head;
    int i = 0;
    while (curr!= NULL  && i <= 10) {
        printf("%ld\t%ld\t%ld\t%ld\n", (addrtype) curr, curr->size,
            curr->is_free,curr->next);
        if (curr->next == NULL) break;
        curr = curr->next;
        i++;
    }
    
}

void *mm_malloc(size_t size) {
    size_t total_size;
	void *block;
	struct header_t *header;
	
	// Size degeri 0 ise NULL dondurulur.
	if (!size)
		return NULL;
		
	// Mutex kilidi sayesinde bu program hafiza aloke ederken hafizaya erisim engellenir.
	pthread_mutex_lock(&global_malloc_lock);
	
	// Blok dizisi, bos ve yeterince buyuk bir blok bulunana kadar bastan sona gezilir.
	struct header_t *curr = head;
	while (curr && (curr->is_free == 0 || curr->size < size)){
		curr = curr->next;
	}
	header = curr;
	
	// Uygun bir blok bulunduysa o blok aloke edilir ve o blokun data kismina isaret eden pointer geri dondurulur.
	if (header) {
		header->is_free = 0;
		pthread_mutex_unlock(&global_malloc_lock);
		return (void*)(header + 1);
	}
	
	// Uygun blok bulunamamissa header ile birlikte toplam gerekli alan hesaplanir ve o kadar yer acilir.
	total_size = sizeof(struct header_t) + size;
	block = sbrk(total_size);
	
	// Yer acilamamissa fonksiyon sonlandirilir ve NULL degeri dondurulur.
	if (block == (void*) -1) {
		pthread_mutex_unlock(&global_malloc_lock);
		return NULL;
	}
	
	// Hazirlanan blok, linkli listeye yerlestirilir.
	header = block;
	header->size = size;
	header->is_free = 0;
	header->next = NULL;
	if (!head)
		head = header;
	if (tail)
		tail->next = header;
	tail = header;
	
	pthread_mutex_unlock(&global_malloc_lock);
	// Header'dan bir byte sonrasi yani data kismini gosteren pointer dondurulur.
	return (void*)(header + 1);
}

void *mm_realloc(void *block, size_t size) {
    struct header_t *header;
	void *ret;
	
	// Fonksiyona gonderilen pointer NULL ise ya da size degeri 0'sa mm_malloc calistirilir.
	if (!block || !size)
		return mm_malloc(size);
	
	// Fonksiyona gonderilen blokun header'i ele alinir.
	header = (struct header_t*)block - 1;
	
	// Blokun buyuklugu realoke edilecek buyuklukten buyuk ya da ona esitse islem yapilmaz, blok oldugu gibi dondurulur.
	if (header->size >= size)
		return block;
		
	// Yeni bir blok olusturulur.
	ret = mm_malloc(size);
	
	// Yeni blokta bir sikinti yoksa eski blokun icerigi yeni bloka kopyalanir ve eski blok free'lenir.
	if (ret) {	
		memcpy(ret, block, header->size);
		free(block);
	}
	
	// Istenilen buyuklukte ve eski blokun tum icerigine de sahip olan yeni blok dondurulur.
	return ret;
}

void mm_free(void *block) {
    struct header_t *header, *tmp;
	void *programbreak;

	// Dealoke edilecek blok zaten aloke edilmemisse fonksiyon sonlandirilir.
	if (!block)
		return;
	
	// Diger programlarin hafizaya erisimi engellenir.	
	pthread_mutex_lock(&global_malloc_lock);
	
	// Dealoke edilecek blokun header'i ele alinir.
	header = (struct header_t*)block - 1;

	// Heap'in mevcut degeri bulunur.
	programbreak = sbrk(0);
	
	// Heap'in mevcut degeriyle fonksiyona gonderilen blokun bitis noktasi karsilastirilir. Degerler esitse bu blok heap'in en sonundadir.
	if ((char*)block + header->size == programbreak) {
		
		// Bu durumda direkt heap'i kucultebiliriz.
		// Bunun icin linkli listede gerekli duzenlemeler yapilir.
		if (head == tail) {
			head = tail = NULL;
		} else {
			tmp = head;
			while (tmp) {
				if(tmp->next == tail) {
					tmp->next = NULL;
					tail = tmp;
				}
				tmp = tmp->next;
			}
		}
		// Linkli liste hazirlandigina gore heap'i fonksiyona gonderilen blok ve blokun header'i buyuklugunde kucultebiliriz.
		sbrk(0 - sizeof(struct header_t) - header->size);
		pthread_mutex_unlock(&global_malloc_lock);
		return;
	}
	
	// Dealoke edilecek blok heap'in tepesinde degilse bu bloku header'inda degisiklik yaparak tekrar aloke edilmeye musait hale getiririz.
	header->is_free = 1;
	pthread_mutex_unlock(&global_malloc_lock);
}
