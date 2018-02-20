#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>
#include <crtdbg.h>
#include <cassert>

namespace sjtu { 

int memory_leak_detector = 0;

template<class T>
class deque {
public:
	struct Node {
		T* data;
		Node *prev, *next;

		Node(const Node& other)
			: prev(other.prev), next(other.next) {
			if (other.data) data = new T(*other.data), memory_leak_detector++;
			else data = NULL;
		}

		Node(Node *prev = NULL, Node *next = NULL, T* _data = NULL)
			: prev(prev), next(next) {
			if (_data) data = new T(*_data), memory_leak_detector++;
			else data = NULL;
		}

		Node(Node *prev, Node *next, const T &_data)
			: prev(prev), next(next) {
			data = new T(_data), memory_leak_detector++;
		}

		~Node() { if (data) delete data, memory_leak_detector--; }
	};

	struct const_iterator;
	struct iterator {
		friend class deque<T>;
	private:
		Node* pointer;
		deque* belong;

	public:
		iterator(const iterator &other)
			: pointer(other.pointer), belong(other.belong) {}

		iterator(Node* ptr = NULL, deque* dq = NULL)
			: pointer(ptr), belong(dq) {}

		iterator operator = (const iterator &other) {
			pointer = other.pointer;
			belong = other.belong;
			return (*this);
		}

		iterator operator + (const int &n) const {
			if (n < 0) return (*this) - (-n);
			Node* ptr = this->pointer;
			for (int i = 0; i < n; i++) {
				ptr = ptr->next;
				if (ptr == NULL) throw invalid_iterator();
			}
			return iterator(ptr, this->belong);
		}

		iterator operator - (const int &n) const {
			if (n < 0) return (*this) + (-n);
			Node* ptr = this->pointer;
			for (int i = 0; i < n; i++) {
				ptr = ptr->prev;
				if (ptr == NULL) throw invalid_iterator();
			}
			return iterator(ptr, this->belong);
		}

		int operator - (const iterator &rhs) const {
			if ((*this).belong != rhs.belong) throw invalid_iterator();

			int ans;
			Node* ptr;

			for (ans = 0, ptr = this->pointer;
				ptr != NULL && ptr != rhs.pointer;
				ptr = ptr->prev, ans++);
			if (ptr == rhs.pointer) return ans;

			for (ans = 0, ptr = this->pointer;
				ptr != NULL && ptr != rhs.pointer;
				ptr = ptr->next, ans--);
			if (ptr == rhs.pointer) return ans;

			throw invalid_iterator();
		}

		iterator operator += (const int &n) { return (*this = *this + n); }
		iterator operator -= (const int &n) { return (*this = *this - n); }

		iterator operator ++ (int) {
			iterator it(*this);
			*this += 1;
			return it;
		}

		iterator & operator++() {
			*this += 1;
			return (*this);
		}

		iterator operator-- (int) {
			iterator it(*this);
			*this -= 1;
			return it;
		}

		iterator & operator--() {
			*this -= 1;
			return (*this);
		}

		T & operator * () const { 
			if (!this->pointer || !this->pointer->data)
				throw invalid_iterator();
			return *(this->pointer->data); 
		}

		T * operator -> () const noexcept { return (this->pointer->data); }

		bool operator == (const iterator &rhs) const {
			return ((*this).belong == rhs.belong) && ((*this).pointer == rhs.pointer);
		}

		bool operator == (const const_iterator &rhs) const {
			return ((*this).belong == rhs.belong) && ((*this).pointer == rhs.pointer);
		}

		bool operator != (const iterator &rhs) const { 
			return ((*this).belong != rhs.belong) || ((*this).pointer != rhs.pointer);
		}

		bool operator != (const const_iterator &rhs) const {
			return ((*this).belong != rhs.belong) || ((*this).pointer != rhs.pointer);
		}
	};

	class const_iterator {
	private:
		iterator it;

	public:
		const_iterator() {}
		const_iterator(const const_iterator &other) : it(other.it) {}
		const_iterator(const iterator &other) : it(other) {}

		const T & operator * () const { return *it; }
		const T * operator -> () const { return &(*it); }
		bool operator == (const const_iterator &rhs) const { return it == rhs.it; }
		bool operator != (const const_iterator &rhs) const { return it != rhs.it; }
		const_iterator operator = (const const_iterator &other) { it = other.it; return (*this); }
		const_iterator operator ++(int) { return const_iterator(it++); }
		const_iterator operator ++() { return const_iterator(++it); }
		const_iterator operator --(int) { return const_iterator(it--); }
		const_iterator operator --() { return const_iterator(--it); }
		const_iterator operator - (const int &n) const { return const_iterator(it - n); }
		const_iterator operator + (const int &n) const { return const_iterator(it + n); }
		const_iterator operator += (const int &n) { return (*this = *this + n); }
		const_iterator operator -= (const int &n) { return (*this = *this - n); }
		int operator - (const const_iterator &rhs) const { return it - rhs.it; }
	};

private:
	size_t size_n;
	iterator head, tail;

public:
	deque() : head(NULL, this), tail(NULL, this), size_n(0) {}
	deque(const deque &other) : head(NULL, this), tail(NULL, this), size_n(0) { (*this) = other; }
	~deque() { 
		clear(); 
		if (head.pointer) delete head.pointer;
	}
	
	void initialize(const T& value) {
		size_n = 1;
		if (head.pointer) delete(head.pointer);
		head.pointer = new Node();
		tail.pointer = new Node();
		head.pointer->data = new T(value), memory_leak_detector++;
		head.pointer->next = tail.pointer;
		tail.pointer->prev = head.pointer;
	}
	
	bool operator == (const deque &rhs) const {
		return (head == rhs.head && tail == rhs.tail);
	}

	bool operator != (const deque &rhs) const { return !(*this == rhs); }

	deque & operator = (const deque &other) {
		if ((*this) != other) {
			this->clear();
			for(Node *ptr = other.head.pointer; ptr && ptr->data; ptr = ptr->next)
				this->push_back(*(ptr->data));
		}
		return (*this);
	}

	T & at(const size_t &pos) {
		if (pos < 0 || pos >= size_n) throw index_out_of_bound();

		Node* pointer = head.pointer;
		for (int i = 0; i < pos; i++) {
			pointer = pointer->next;
		}
		return *(pointer->data);
	}

	const T & at(const size_t &pos) const {
		if (pos < 0 || pos >= size_n) throw index_out_of_bound();

		Node* pointer = head.pointer;
		for (int i = 0; i < pos; i++) {
			pointer = pointer->next;
		}
		return *(pointer->data);
	}

	T & operator [] (const size_t &pos) { return this->at(pos); }
	const T & operator [] (const size_t &pos) const { return this->at(pos); }

	const T & front() const {
		if (this->empty()) throw container_is_empty();
		return *(head.pointer->data);
	}

	const T & back() const {
		if (this->empty()) throw container_is_empty();
		return *(tail.pointer->prev->data);
	}

	iterator begin() { return head; }
	const_iterator cbegin() const { return head; }
	iterator end() { return tail; }
	const_iterator cend() const { return tail; }

	bool empty() const { return (size_n == 0); }

	size_t size() const { return size_n; }

	void clear() {
		while (!empty()) pop_back();
	}

	iterator insert(iterator pos, const T &value) {
		if (pos.belong != this) throw invalid_iterator();
		if (empty()) {
			push_back(value);
			return head;
		}
		if (!pos.pointer) throw invalid_iterator();

		// custom
		Node* &ptr = pos.pointer;
		Node* new_node = new Node(ptr->prev, ptr, value);
		if (ptr->prev) ptr->prev->next = new_node;
		ptr->prev = new_node;
		size_n++;
		if (pos == head) --head;
		return iterator(new_node, this);
	}

	iterator erase(iterator pos) {
		if (!pos.pointer || !pos.pointer->data || pos.belong != this) {
			throw invalid_iterator();
		}
		Node* ptr = pos.pointer;
		Node* nxt = ptr->next;
		if (ptr->prev) ptr->prev->next = ptr->next;
		if (ptr->next) ptr->next->prev = ptr->prev;
		if (pos == head) head++;
		delete ptr;
		size_n--;

		return iterator(nxt, this);
	}

	void push_back(const T &value) { 
		if (empty()) initialize(value);
		else insert(tail, value); 
	}

	void push_front(const T &value) {
		if (empty()) initialize(value);
		else insert(head, value);
	}

	void pop_back() { tail = erase(tail - 1); }
	void pop_front() { head = erase(head); }
};

}

#endif