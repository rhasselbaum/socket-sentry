/***************************************************************************
 *   Copyright (C) 2010 by Rob Hasselbaum <rob@hasselbaum.net>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/


#ifndef TESTMAIN_H_
#define TESTMAIN_H_

#include <QtTest/QtTest>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

/*
 * Project specific macro for main function in unit tests.
 */

#define QTEST_GMOCK_MAIN(TestObject) \
int main(int argc, char *argv[]) { \
    ::testing::GTEST_FLAG(throw_on_failure) = true; \
    ::testing::InitGoogleMock(&argc, argv); \
    QCoreApplication app(argc, argv); \
    TestObject tc; \
    return QTest::qExec(&tc, argc, argv); \
}

#endif /* TESTMAIN_H_ */
