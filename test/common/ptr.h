#include <cxxtest/TestSuite.h>

#include "common/ptr.h"

class PtrTestSuite : public CxxTest::TestSuite {
	public:

	struct A {
		int a;
	};

	struct B : public A {
		int b;
	};

	// A simple class which keeps track of all its instances
	class InstanceCountingClass {
	public:
		static int count;
		InstanceCountingClass() { count++; }
		InstanceCountingClass(const InstanceCountingClass&) { count++; }
		~InstanceCountingClass() { count--; }
	};

	void test_deletion() {
		TS_ASSERT_EQUALS(InstanceCountingClass::count, 0);
		{
			Common::SharedPtr<InstanceCountingClass> p1(new InstanceCountingClass());
			TS_ASSERT_EQUALS(InstanceCountingClass::count, 1);

			Common::ScopedPtr<InstanceCountingClass> p2(new InstanceCountingClass());
			TS_ASSERT_EQUALS(InstanceCountingClass::count, 2);
		}
		TS_ASSERT_EQUALS(InstanceCountingClass::count, 0);
	}

	struct CustomDeleter {
		static bool invoked;
		void operator()(int *object) {
			invoked = true;
			delete object;
		}
	};

	void test_scoped_deleter() {
		CustomDeleter::invoked = false;

		{
			Common::ScopedPtr<int, CustomDeleter> a(new int(0));
			TS_ASSERT(!CustomDeleter::invoked);
		}

		TS_ASSERT(CustomDeleter::invoked);
	}

	void test_disposable_deleter() {
		CustomDeleter::invoked = false;

		{
			Common::DisposablePtr<int, CustomDeleter> a1(new int, DisposeAfterUse::YES);
			TS_ASSERT(!CustomDeleter::invoked);
		}

		TS_ASSERT(CustomDeleter::invoked);
		CustomDeleter::invoked = false;

		int *a = new int;
		{
			Common::DisposablePtr<int, CustomDeleter> a2(a, DisposeAfterUse::NO);
		}

		TS_ASSERT(!CustomDeleter::invoked);
		delete a;
	}

	void test_scoped_deref() {
		A *raw = new A();
		raw->a = 123;
		Common::ScopedPtr<A> a(raw);
		TS_ASSERT_EQUALS(&*a, &*raw);
		TS_ASSERT_EQUALS(a->a, raw->a);
	}

	void test_disposable_deref() {
		A *raw = new A();
		raw->a = 123;
		Common::DisposablePtr<A> a(raw, DisposeAfterUse::YES);
		TS_ASSERT_EQUALS(&*a, &*raw);
		TS_ASSERT_EQUALS(a->a, raw->a);
	}

	void test_assign() {
		Common::SharedPtr<int> p1(new int(1));
		TS_ASSERT(p1.unique());
		TS_ASSERT_EQUALS(*p1, 1);

		{
			Common::SharedPtr<int> p2 = p1;
			TS_ASSERT(!p1.unique());
			TS_ASSERT_EQUALS(p1.refCount(), p2.refCount());
			TS_ASSERT_EQUALS(p1.refCount(), 2);
			TS_ASSERT_EQUALS(p1, p2);
			TS_ASSERT_EQUALS(*p2, 1);
			{
				Common::SharedPtr<int> p3;
				p3 = p2;
				TS_ASSERT_EQUALS(p3, p2);
				TS_ASSERT_EQUALS(p3, p1);
				TS_ASSERT_EQUALS(p1.refCount(), 3);
				TS_ASSERT_EQUALS(*p3, 1);
				*p3 = 0;
				TS_ASSERT_EQUALS(*p3, 0);
			}
			TS_ASSERT_EQUALS(*p2, 0);
			TS_ASSERT_EQUALS(p1.refCount(), 2);
		}

		TS_ASSERT_EQUALS(*p1, 0);
		TS_ASSERT(p1.unique());
	}

	template<class T>
	struct Deleter {
		bool *test;
		void operator()(T *ptr) { *test = true; delete ptr; }
	};

	void test_deleter() {
		Deleter<int> myDeleter;
		bool test = false;
		myDeleter.test = &test;

		{
			Common::SharedPtr<int> p(new int(1), myDeleter);
		}

		TS_ASSERT_EQUALS(test, true);
	}

	void test_compare() {
		Common::SharedPtr<int> p1(new int(1));
		Common::SharedPtr<int> p2;

		TS_ASSERT(p1);
		TS_ASSERT(!p2);

		TS_ASSERT(p1 != 0);
		TS_ASSERT(p2 == 0);

		p1.reset();
		TS_ASSERT(!p1);
	}

	void test_cast() {
		Common::SharedPtr<B> b(new B);
		Common::SharedPtr<A> a(b);
		a = b;
	}

	void test_weak_ptr() {
		Common::SharedPtr<B> b(new B);
		Common::WeakPtr<A> a(b);
		TS_ASSERT(a.lock() == b);
		TS_ASSERT(!a.expired());
		b.reset();
		TS_ASSERT(a.expired());
		TS_ASSERT(!a.lock());
	}

	struct IsDeletedClass {
		bool isDeleted = false;
		void operator ()(IsDeletedClass *ptr) {
			TS_ASSERT(!ptr->isDeleted);
			ptr->isDeleted = true;
		}
	};

	void test_disposable_move_assign() {
		IsDeletedClass obj1, obj2, obj3, obj4;
		Common::DisposablePtr<IsDeletedClass, IsDeletedClass> p1(&obj1, DisposeAfterUse::YES);

		// move-assign from YES to YES ptr
		{
			Common::DisposablePtr<IsDeletedClass, IsDeletedClass> p2(&obj2, DisposeAfterUse::YES);
			p1 = Common::move(p2);
			TS_ASSERT(obj1.isDeleted);
		}
		TS_ASSERT(!obj2.isDeleted); // p2 did not delete its old object

		// move-assign from NO to YES ptr
		{
			Common::DisposablePtr<IsDeletedClass, IsDeletedClass> p3(&obj3, DisposeAfterUse::NO);
			p1 = Common::move(p3);
			TS_ASSERT(obj2.isDeleted);
		}
		p1.reset();
		TS_ASSERT(!obj3.isDeleted); // NO dispose flag was correctly propagated

		// move-assign from YES to NO ptr
		{
			Common::DisposablePtr<IsDeletedClass, IsDeletedClass> p4(&obj4, DisposeAfterUse::YES);
			p1 = Common::move(p4);
			TS_ASSERT(!obj3.isDeleted); // this was still NO
			TS_ASSERT(!obj4.isDeleted);
		}
		p1.reset();
		TS_ASSERT(obj4.isDeleted); // YES dispose flag was correctly propagated
	}
};

int PtrTestSuite::InstanceCountingClass::count = 0;
bool PtrTestSuite::CustomDeleter::invoked = false;
