/* 
 * File:   AlarmClockTest.cpp
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#include "AlarmClockTest.h"
#include "AlarmClock.h"
#include "StopWatch.h"
#include <chrono>

std::atomic<unsigned int> AlarmClockTest::mFakeSleepUs(0);

namespace {
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;

   unsigned int kFakeSleepLeeway = 100;

   template<typename T>
   void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
      while (!alerter.Expired());
   }

   template<typename Duration>
   unsigned int ConvertToMicroSeconds(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }
   
   template<typename Duration>
   unsigned int ConvertToMilliSeconds(Duration t) {
      return std::chrono::duration_cast<milliseconds>(t).count();
   }

   void FakeSleep(unsigned int usToSleep) {
      AlarmClockTest::mFakeSleepUs.store(usToSleep); 
   }
}

TEST_F(AlarmClockTest, GetUsSleepTimeInUs) {
   int us = 123456;
   AlarmClock<microseconds> alerter(us);
   EXPECT_EQ(us, alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetUsSleepTimeInMs) {
   int us = 123456;
   AlarmClock<microseconds> alerter(us);
   EXPECT_EQ(ConvertToMilliSeconds(microseconds(us)), alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, GetMsSleepTimeInMs) {
   int ms = 123456;
   AlarmClock<milliseconds> alerter(ms);
   EXPECT_EQ(ms, alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, GetMsSleepTimeInUs) {
   int ms = 123456;
   AlarmClock<milliseconds> alerter(ms);
   EXPECT_EQ(ConvertToMicroSeconds(milliseconds(ms)), alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetSecSleepTimeInUs) {
   int sec = 1;
   AlarmClock<seconds> alerter(sec);
   EXPECT_EQ(ConvertToMicroSeconds(seconds(sec)), alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetSecSleepTimeInMs) {
   int sec = 1;
   AlarmClock<seconds> alerter(sec);
   EXPECT_EQ(ConvertToMilliSeconds(seconds(sec)), alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, microsecondsLessThan500ms) {
   int us = 900;
   AlarmClock<microseconds> alerter(us, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, us-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, us);
}

TEST_F(AlarmClockTest, microsecondsGreaterThan500ms) {
   int us = 600000;
   AlarmClock<microseconds> alerter(us, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, us-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, us);
}

TEST_F(AlarmClockTest, weirdNumberOfMicroseconds) {
   int us = 724509;
   StopWatch sw;
   AlarmClock<microseconds> alerter(us, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, us-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, us);
}

TEST_F(AlarmClockTest, millisecondsLessThan500) {
   unsigned int ms = 100;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs()-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, oneSecondInMilliseconds) {
   unsigned int ms = 1000;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs()-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, millisecondsNotDivisibleBy500) {
   unsigned int ms = 1000;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs()-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, secondsSimple) {
   unsigned int sec = 1;
   AlarmClock<seconds> alerter(sec, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs()-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, LongTimeout_ImmediatelyDestructed) {
   unsigned int sec = 1000;
   StopWatch sw;
   std::unique_ptr<AlarmClock<seconds>> acPtr(new AlarmClock<seconds>(sec, FakeSleep));
   EXPECT_FALSE(acPtr->Expired());
   acPtr.reset();
   EXPECT_TRUE(sw.ElapsedSec() < 2);
}

TEST_F(AlarmClockTest, milliseconds_ResetAfterExpired) {
   // First run
   int ms = 750;
   StopWatch testTimer;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   
   // Reset after AlarmClock has expired
   alerter.Reset();
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
}

TEST_F(AlarmClockTest, milliseconds_ResetBeforeExpired) {
   int ms = 7500;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   alerter.Reset();
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
}