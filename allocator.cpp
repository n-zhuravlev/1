#include "allocator.h"

Pointer Allocator::alloc(size_t N)
{
	
	if(Managing_List.empty()) {
		Managing_List.push_back(Memory_Element(Start_Ptr, N));
		return Pointer(Managing_List.begin());
	}
	
	list<Memory_Element>::iterator current_iter = Managing_List.begin(), next_iter = ++Managing_List.begin();
	while(next_iter != Managing_List.end()) {
		if(next_iter->ptr - (current_iter->ptr + current_iter->len) >= N) return Pointer(Managing_List.insert(next_iter, Memory_Element(current_iter->ptr + current_iter->len, N)));	
		current_iter++;
		next_iter++;
	}
	if(Start_Ptr + Mem_size - (current_iter->ptr + current_iter->len) >= N) {
		Managing_List.push_back(Memory_Element(current_iter->ptr + current_iter->len, N));
		return Pointer(--Managing_List.end());
	}
	throw AllocError(AllocErrorType::NoMemory, "NoMemory");	
	
}


void *Pointer::get() const{
	if(if_NULL) return NULL;
	return (void *)iter->ptr ;	

}

void Allocator::realloc(Pointer &p, size_t N)
{
	if(p.if_NULL) {
		p = alloc(N);
		return;
	}
	list<Memory_Element>::iterator next = p.iter;
	next++;
	if(N <= p.iter->len || (next != Managing_List.end() && (next->ptr - p.iter->ptr >= N))) {
		p.iter->len = N;
		return;
	}
	list<Memory_Element>::iterator current_iter = Managing_List.begin(), next_iter = ++Managing_List.begin();
	while(next_iter != Managing_List.end()) {
		if(next_iter->ptr - (current_iter->ptr + current_iter->len) >= N) {
			int8_t *new_ptr = current_iter->ptr + current_iter->len, *current_ptr = p.iter->ptr;
			while(current_ptr < p.iter->ptr + p.iter->len) *new_ptr++ = *current_ptr++;
			Managing_List.erase(p.iter);
			p.iter = Managing_List.insert(next_iter, Memory_Element(current_iter->ptr + current_iter->len, N));		
			return;
		}	
		current_iter++;
		next_iter++;
	}
	if(current_iter == p.iter && Start_Ptr + Mem_size - current_iter->ptr >= N) {
		p.iter->len = N;
		return;
	}
	
	if(Start_Ptr + Mem_size - (current_iter->ptr + current_iter->len) >= N) {
		int8_t *new_ptr = current_iter->ptr + current_iter->len, *current_ptr = p.iter->ptr;
		while(current_ptr < p.iter->ptr + p.iter->len) *new_ptr++ = *current_ptr++;
		Managing_List.erase(p.iter);
		Managing_List.push_back(Memory_Element(current_iter->ptr + current_iter->len, N));	
		p.iter = --Managing_List.end();
		return;
	}
	throw AllocError(AllocErrorType::NoMemory, "NoMemory");

}

void Allocator::free(Pointer &p)
{
	if(p.if_NULL == true) {
		throw AllocError(AllocErrorType::InvalidFree, "InvalidFree");
	}
	Managing_List.erase(p.iter);
	p.if_NULL = true;
	return;
}

void Allocator::defrag()
{
	int8_t *current_ptr, *new_ptr = Start_Ptr, *save;
	list<Memory_Element>::iterator current_iter = Managing_List.begin();
	while(current_iter != Managing_List.end()) {
		current_ptr = current_iter->ptr;
		save = new_ptr;
		while(current_ptr < current_iter->ptr + current_iter->len) *new_ptr++ = *current_ptr++;
		current_iter->ptr = save;
		current_iter++;
	}
	return;
}





















