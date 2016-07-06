#include <iostream>
#include <sstream>

#define BOOST_TEST_MODULE example
#include <boost/test/included/unit_test.hpp>

#include "regilo/scanrecord.hpp"
#include "regilo/scandata.hpp"

struct ScanRecordDataFixture
{
	int id1 = 0;
	double angle1 = 0.1;
	double distance1 = 1342;
	int intensity1 = 123;
	int errorCode1 = 0;
	bool error1 = false;

	int id2 = 1;
	double angle2 = 0.2;
	double distance2 = 1023.43;
	int intensity2 = 9;
	int errorCode2 = 3;
	bool error2 = true;

	std::size_t scanId = 4;
	double rotationSpeed = 25.6;

	regilo::ScanRecord record1;
	regilo::ScanRecord record2;

	regilo::ScanData data;

	ScanRecordDataFixture() :
		record1(id1, angle1, distance1, intensity1, errorCode1, error1),
		record2(id2, angle2, distance2, intensity2, errorCode2, error2),
		data(scanId, rotationSpeed)
	{
		data.push_back(record1);
		data.push_back(record2);
	}
};

BOOST_FIXTURE_TEST_SUITE(ScanRecordDataSuite, ScanRecordDataFixture)

BOOST_AUTO_TEST_CASE(ScanRecordConstructorValues)
{
	BOOST_REQUIRE(record1.id == id1);
	BOOST_REQUIRE(record1.angle == angle1);
	BOOST_REQUIRE(record1.distance == distance1);
	BOOST_REQUIRE(record1.intensity == intensity1);
	BOOST_REQUIRE(record1.errorCode == errorCode1);
	BOOST_REQUIRE(record1.error == error1);
}

BOOST_AUTO_TEST_CASE(ScanRecordPrint)
{
	std::ostringstream out, correct;
	out << record1 << std::endl << record2 << std::endl;

	correct << "ScanRecord(" << id1 << ": " << angle1 * 180 * M_1_PI << "°; " << distance1 << "mm)" << std::endl;
	correct << "ScanRecord(" << id2 << ": " << angle2 * 180 * M_1_PI << "°; " << distance2 << "mm; error)" << std::endl;

	BOOST_REQUIRE(out.str() == correct.str());
}

BOOST_AUTO_TEST_CASE(ScanDataDefaultValues)
{
	regilo::ScanData defaultData;

	BOOST_REQUIRE(defaultData.scanId == std::size_t(-1));
	BOOST_REQUIRE(defaultData.rotationSpeed == double(-1));
}

BOOST_AUTO_TEST_CASE(ScanDataConstructorValues)
{
	BOOST_REQUIRE(data.scanId == scanId);
	BOOST_REQUIRE(data.rotationSpeed == rotationSpeed);
}

BOOST_AUTO_TEST_CASE(ScanDataPrint)
{
	std::ostringstream out, correct;
	out << data;

	correct << "ScanData(" << scanId << ", " << rotationSpeed << ", " << 2 << ')' << std::endl;
	correct << "ScanRecord(" << id1 << ": " << angle1 * 180 * M_1_PI << "°; " << distance1 << "mm)" << std::endl;
	correct << "ScanRecord(" << id2 << ": " << angle2 * 180 * M_1_PI << "°; " << distance2 << "mm; error)" << std::endl;

	BOOST_REQUIRE(out.str() == correct.str());
}

BOOST_AUTO_TEST_SUITE_END()
