/*******************************************************************************
 * Copyright IBM Corp. and others 2024
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0-only WITH Classpath-exception-2.0 OR GPL-2.0-only WITH OpenJDK-assembly-exception-1.0
 *******************************************************************************/

#include "gtest/gtest.h"
#include "ilgen/IlInjector.hpp"
#include "ilgen/IlGeneratorMethodDetails_inlines.hpp"
#include "ilgen/MethodInfo.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "OptTestDriver.hpp"

namespace TestCompiler
{

/**
 * Simple IL Injector that generates: return param1 + param2
 */
class SimpleAddIlInjector : public TR::IlInjector
   {
   public:
   TR_ALLOC(TR_Memory::IlGenerator)

   SimpleAddIlInjector(TR::TypeDictionary *types, TestDriver *test)
      : TR::IlInjector(types, test)
      {
      }

   bool injectIL()
      {
      createBlocks(1);
      
      // return parameter(0) + parameter(1);
      returnValue(
         createWithoutSymRef(TR::iadd, 2,
            parameter(0, Int32),
            parameter(1, Int32)));
      
      return true;
      }
   };

/**
 * Method info describing the signature: int32_t add(int32_t a, int32_t b)
 */
class SimpleAddInfo : public TestCompiler::MethodInfo
   {
   public:
   SimpleAddInfo(TestDriver *test)
      : _ilInjector(&_types, test)
      {
      TR::IlType* Int32 = _types.PrimitiveType(TR::Int32);
      _args[0] = Int32;
      _args[1] = Int32;
      DefineFunction(__FILE__, LINETOSTR(__LINE__), "simpleAdd", 2, _args, Int32);
      DefineILInjector(&_ilInjector);
      }

   typedef int32_t (*MethodType)(int32_t, int32_t);

   private:
   TR::TypeDictionary _types;
   TestCompiler::SimpleAddIlInjector _ilInjector;
   TR::IlType *_args[2];
   };

/**
 * Test driver for simple addition test
 */
class SimpleAddTest : public OptTestDriver
   {
   public:
   void invokeTests()
      {
      auto testMethod = getCompiledMethod<SimpleAddInfo::MethodType>();
      
      // Test basic addition
      ASSERT_EQ(5, testMethod(2, 3));
      ASSERT_EQ(0, testMethod(0, 0));
      ASSERT_EQ(10, testMethod(7, 3));
      ASSERT_EQ(-5, testMethod(-2, -3));
      ASSERT_EQ(0, testMethod(5, -5));
      ASSERT_EQ(100, testMethod(50, 50));
      }
   };

/**
 * The actual test case
 */
TEST_F(SimpleAddTest, BasicAddition)
   {
   SimpleAddInfo info(this);
   setMethodInfo(&info);
   
   // Compile and invoke the test
   VerifyAndInvoke();
   }

} // namespace TestCompiler