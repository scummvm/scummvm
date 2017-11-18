#include <cxxtest/TestSuite.h>
#include <typeinfo>

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

	void test_scoped_array() {
		Common::ScopedPtr<int[]> a(new int[10]);
		Common::ScopedPtr<int[], Common::DefaultDeleter<int[]> > rightA(nullptr);
		TS_ASSERT(typeid(a) == typeid(rightA));

		a[0] = 123;
		TS_ASSERT_EQUALS(a[0], 123);

		Common::ScopedPtr<const int[]> b(new int[10]);
		Common::ScopedPtr<const int[], Common::DefaultDeleter<const int[]> > rightB(nullptr);
		TS_ASSERT(typeid(b) == typeid(rightB));

		a[0] = 1234;
	}

	void test_disposable_array() {
		Common::DisposablePtr<int[]> a(new int[10], DisposeAfterUse::YES);
		Common::DisposablePtr<int[], Common::DefaultDeleter<int[]> > rightA(nullptr, DisposeAfterUse::YES);
		TS_ASSERT(typeid(a) == typeid(rightA));

		a[0] = 123;
		TS_ASSERT_EQUALS(a[0], 123);

		Common::DisposablePtr<const int[]> b(new int[10], DisposeAfterUse::YES);
		Common::DisposablePtr<const int[], Common::DefaultDeleter<const int[]> > rightB(nullptr, DisposeAfterUse::YES);
		TS_ASSERT(typeid(b) == typeid(rightB));

		a[0] = 1234;
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
		myDeleter.test = new bool(false);

		{
			Common::SharedPtr<int> p(new int(1), myDeleter);
		}

		TS_ASSERT_EQUALS(*myDeleter.test, true);
		delete myDeleter.test;
	}

	void test_shared_array() {
		Common::SharedPtr<int[]> p1(new int[10]);
		Common::SharedPtr<int[]> p2 = p1;
		p1[0] = 123;
		TS_ASSERT_EQUALS(p1[0], 123);
		TS_ASSERT_EQUALS(p2[0], 123);
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
};

int PtrTestSuite::InstanceCountingClass::count = 0;
bool PtrTestSuite::CustomDeleter::invoked = false;
