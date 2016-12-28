/*
 * Regilo
 * Copyright (C) 2015-2016  Branislav Holý <branoholy@gmail.com>
 *
 * This file is part of Regilo.
 *
 * Regilo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Regilo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Regilo.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cmath>
#include <iostream>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include "regilo/scandata.hpp"

struct ScanRecordDataFixture
{
    int index1 = 0;
    double angle1 = 0.1;
    double distance1 = 1342;
    int intensity1 = 123;
    int errorCode1 = 0;
    bool error1 = false;

    int index2 = 1;
    double angle2 = 0.2;
    double distance2 = 1023.43;
    int intensity2 = 9;
    int errorCode2 = 3;
    bool error2 = true;

    std::size_t scanIndex= 4;
    double rotationSpeed = 25.6;

    regilo::ScanRecord record1;
    regilo::ScanRecord record2;

    regilo::ScanData data;

    ScanRecordDataFixture() :
        record1(index1, angle1, distance1, intensity1, errorCode1, error1),
        record2(index2, angle2, distance2, intensity2, errorCode2, error2),
        data(scanIndex, rotationSpeed)
    {
        data.push_back(record1);
        data.push_back(record2);
    }
};

BOOST_FIXTURE_TEST_SUITE(ScanRecordDataSuite, ScanRecordDataFixture)

BOOST_AUTO_TEST_CASE(ScanRecordConstructorValues)
{
    BOOST_REQUIRE(record1.index == index1);
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

    correct << "ScanRecord("
            << index1 << ": "
            << angle1 * 180 * M_1_PI << "°; "
            << distance1 << "mm)"
            << std::endl;

    correct << "ScanRecord("
            << index2 << ": "
            << angle2 * 180 * M_1_PI << "°; "
            << distance2 << "mm; error)"
            << std::endl;

    BOOST_REQUIRE(out.str() == correct.str());
}

BOOST_AUTO_TEST_CASE(ScanDataDefaultValues)
{
    regilo::ScanData defaultData;

    BOOST_REQUIRE(defaultData.index == std::size_t(-1));
    BOOST_REQUIRE(defaultData.rotationSpeed == double(-1));
}

BOOST_AUTO_TEST_CASE(ScanDataConstructorValues)
{
    BOOST_REQUIRE(data.index == scanIndex);
    BOOST_REQUIRE(data.rotationSpeed == rotationSpeed);
}

BOOST_AUTO_TEST_CASE(ScanDataPrint)
{
    std::ostringstream out, correct;
    out << data;

    correct << "ScanData("
            << scanIndex << ", "
            << rotationSpeed << ", "
            << 2 << ')'
            << std::endl;

    correct << "ScanRecord("
            << index1 << ": "
            << angle1 * 180 * M_1_PI << "°; "
            << distance1 << "mm)"
            << std::endl;

    correct << "ScanRecord("
            << index2 << ": "
            << angle2 * 180 * M_1_PI << "°; "
            << distance2 << "mm; error)"
            << std::endl;

    BOOST_REQUIRE(out.str() == correct.str());
}

BOOST_AUTO_TEST_SUITE_END()
