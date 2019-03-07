/* -------------------------------------------------------------------------- *
 *                       OpenSim:  testXsensDataReader.cpp                    *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2017 Stanford University and the Authors                *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "OpenSim/Common/DataAdapter.h"
#include "OpenSim/Common/MapObject.h"
#include "OpenSim/Common/XsensDataReader.h"
#include "OpenSim/Common/STOFileAdapter.h"
#include <OpenSim/Auxiliary/auxiliaryTestFunctions.h>


using namespace OpenSim;
/* Raw data from 00B421AF
PacketCounter<tab>SampleTimeFine<tab>Year<tab>Month<tab>Day<tab>Second<tab>UTC_Nano<tab>UTC_Year<tab>UTC_Month<tab>UTC_Day<tab>UTC_Hour<tab>UTC_Minute<tab>UTC_Second<tab>UTC_Valid<tab>Acc_X<tab>Acc_Y<tab>Acc_Z<tab>Gyr_X<tab>Gyr_Y<tab>Gyr_Z<tab>Mag_X<tab>Mag_Y<tab>Mag_Z<tab>Mat[1][1]<tab>Mat[2][1]<tab>Mat[3][1]<tab>Mat[1][2]<tab>Mat[2][2]<tab>Mat[3][2]<tab>Mat[1][3]<tab>Mat[2][3]<tab>Mat[3][3]
03583<tab><tab><tab><tab><tab><tab><tab><tab><tab><tab><tab><tab><tab><tab>3.030769<tab>5.254238<tab>-7.714005<tab>0.005991<tab>-0.032133<tab>0.022713<tab>-0.045410<tab>-0.266113<tab>0.897217<tab>0.609684<tab>0.730843<tab>0.306845<tab>0.519480<tab>-0.660808<tab>0.541732<tab>0.598686<tab>-0.170885<tab>-0.782543
...
06951<tab><tab><tab><tab><tab><tab><tab><tab><tab><tab><tab><tab><tab><tab>2.657654<tab>5.012634<tab>-7.581414<tab>0.392058<tab>0.353193<tab>-0.712047<tab>-0.114258<tab>-0.155518<tab>0.913330<tab>0.387756<tab>0.877453<tab>0.282349<tab>0.716718<tab>-0.479624<tab>0.506238<tab>0.579621<tab>0.006068<tab>-0.814863
*/

int main() {

    try {
        MapObject mapOpenSimIMUToFileName;
        std::vector<std::string> imu_names{ "shank"};
        std::vector<std::string> file_names{ "000_00B421AF"};
        // Programmatically add items to Map, write to xml
        int index = 0;
        for (auto name : imu_names) {
            MapItem newItem(name, file_names[index]);
            mapOpenSimIMUToFileName.addItem(newItem);
            index++;
        }
        mapOpenSimIMUToFileName.print("map2xml.xml");
        // read xml we wrote into a new MapObject and pass to readXsensTrial
        MapObject readMapIMUName2FileName("map2xml.xml");
        const std::string folder = "";
        const std::string trial = "MT_012005D6_031-";
        DataAdapter::OutputTables tables = XsensDataReader::readTrial(folder, trial, readMapIMUName2FileName);
        // Write tables to sto files
        // Accelerations
        std::shared_ptr<AbstractDataTable> accelTable = tables.at(XsensDataReader::LinearAccelerations);
        const TimeSeriesTableVec3& accelTableTyped = dynamic_cast<const TimeSeriesTableVec3&>(*accelTable);
        STOFileAdapterVec3::write(accelTableTyped, folder + trial+ "accelerations.sto");
        const SimTK::RowVectorView_<SimTK::Vec3>& rvv = accelTableTyped.getRowAtIndex(0);
        SimTK::Vec3 fromTable = accelTableTyped.getRowAtIndex(0)[0];
        SimTK::Vec3 fromFile = SimTK::Vec3{ 3.030769, 5.254238, -7.714005 };
        double tolerance = SimTK::Eps;
        ASSERT_EQUAL(fromTable,fromFile, tolerance);
        // test last row as well to make sure all data is read correctly, size is as expected
        int numRows = accelTableTyped.getIndependentColumn().size();
        fromTable = accelTableTyped.getRowAtIndex(numRows-1)[0];
        fromFile = SimTK::Vec3{ 2.657654, 5.012634, -7.581414 };
        ASSERT_EQUAL(fromTable, fromFile, tolerance);
        // Magenometer
        std::shared_ptr<AbstractDataTable> magTable = tables.at(XsensDataReader::MagneticHeading);
        const TimeSeriesTableVec3& magTableTyped = dynamic_cast<const TimeSeriesTableVec3&>(*magTable);
        STOFileAdapterVec3::write(magTableTyped, folder + trial + "magnetometers.sto");
        fromTable = magTableTyped.getRowAtIndex(0)[0];
        fromFile = SimTK::Vec3{ -0.045410, - 0.266113, 0.897217 };
        ASSERT_EQUAL(fromTable, fromFile, tolerance);
        // Gyro
        std::shared_ptr<AbstractDataTable> gyroTable = tables.at(XsensDataReader::AngularVelocity);
        const TimeSeriesTableVec3& gyroTableTyped = dynamic_cast<const TimeSeriesTableVec3&>(*gyroTable);
        STOFileAdapterVec3::write(gyroTableTyped, folder + trial + "gyros.sto");
        fromTable = gyroTableTyped.getRowAtIndex(0)[0];
        fromFile = SimTK::Vec3{ 0.005991, - 0.032133, 0.022713 };
        ASSERT_EQUAL(fromTable, fromFile, tolerance);
        // Orientation
        std::shared_ptr<AbstractDataTable> orientationTable = tables.at(XsensDataReader::Orientations);
        const TimeSeriesTableQuaternion& quatTableTyped = dynamic_cast<const TimeSeriesTableQuaternion&>(*orientationTable);
        STOFileAdapterQuaternion::write(quatTableTyped, folder + trial + "quaternions.sto");

        // Now test the case where only orintation data is available, rest is missing
        MapObject mapOpenSimIMUToFileNameOrientationOnly;
        MapItem newItem("test", "000_00B421ED");
        mapOpenSimIMUToFileNameOrientationOnly.addItem(newItem);
        DataAdapter::OutputTables tables2 = XsensDataReader::readTrial("", "MT_012005D6-000_sit_to_stand-", mapOpenSimIMUToFileNameOrientationOnly);
        std::shared_ptr<AbstractDataTable> accelTable2 = tables2.at(XsensDataReader::LinearAccelerations);
        int tableSize = accelTable2->getNumRows();
        ASSERT(tableSize ==0);
    }
    catch (const std::exception& ex) {
        std::cout << "testXsensDataReader FAILED: " << ex.what() << std::endl;
        return 1;
    }

    std::cout << "\n All testXsensDataReader cases passed." << std::endl;

    return 0;
}
