#define BOOST_TEST_MODULE DataType
#include <boost/test/unit_test.hpp>
#include "../../src/dataType/DataValueLong.h"

using namespace storage;
namespace unitTests {
	BOOST_AUTO_TEST_SUITE(DataTypeTest)

	BOOST_AUTO_TEST_CASE(DataValueLong_test)
	{
		DataValueLong dv1;
		DataValueLong dv4(dv1);
		DataValueLong dv5 = dv1;

		BOOST_TEST(dv1.GetDataType() == DataType::LONG);
		BOOST_TEST(dv1.GetValueType() == ValueType::NULL_VALUE);
		BOOST_TEST(dv1.IsFixLength());
		BOOST_TEST(dv1.IsNull());
		BOOST_TEST(dv1.GetMaxLength() == 8);
		BOOST_TEST(dv1.GetLength(true) == 8);
		BOOST_TEST(dv1.GetLength(false) == 1);
		BOOST_TEST(dv1 == dv5);
		BOOST_TEST(dv1 == dv4);

		DataValueLong dv2(2);
		BOOST_TEST(dv2.GetDataType() == DataType::LONG);
		BOOST_TEST(dv2.GetValueType() == ValueType::SOLE_VALUE);
		BOOST_TEST(!dv2.IsNull());
		BOOST_TEST(dv2.GetLength(true) == 8);
		BOOST_TEST(dv2.GetLength(false) == 9);
		BOOST_TEST(dv1 < dv2);
		BOOST_TEST(dv1 <= dv2);
		BOOST_TEST(!(dv1 == dv2));

		//dv4 = dv2;
		//BOOST_TEST(dv4 == dv2);

		//char buf[100];
		//*((int*)buf) = 10;
		//DataValueLong dv3(buf, 0);
		//BOOST_TEST(dv3.GetValueType() == ValueType::BYTES_VALUE);

		//BOOST_TEST(dv3 > dv2);
		//BOOST_TEST(dv3 >= dv2);
		//BOOST_TEST(dv3 != dv2);

		//dv1.SetDefaultValue();
		//BOOST_TEST(dv1 == 0);

		//dv1.SetMaxValue();
		//BOOST_TEST(dv1 == LLONG_MAX);

		//dv1.SetMinValue();
		//BOOST_TEST(dv1 == LLONG_MIN);
	}
	BOOST_AUTO_TEST_SUITE_END()
}