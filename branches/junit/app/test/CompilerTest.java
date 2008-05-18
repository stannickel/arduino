//
//  CompilerTest.java
//  Arduino
//
//  Created by David A. Mellis on 5/10/08.
//  Copyright 2008 David A. Mellis. All rights reserved.
//

package processing.app.test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.junit.Test;
import org.junit.Before;
import org.junit.Assert;
import static org.junit.Assert.*;

import processing.app.Compiler;
import processing.app.Preferences;

public class CompilerTest {
  /** http://mindview.net/WebLog/log-0054 */
  private void compare(List lst, String[] strs) {
    Object[] array = lst.toArray();
    assertTrue("Arrays not the same length", array.length == strs.length);
    for(int i = 0; i < array.length; i++)
      assertEquals(strs[i], (String)array[i]);
  }

  @Before public void setUp() {
    Preferences.set("board", "test");
    Preferences.set("boards.test.build.mcu", "testmcu");
    Preferences.set("boards.test.build.f_cpu", "testf_cpu");
  }
  
  @Test public void testGetCommandCompilerC() {
    compare(
      Compiler.getCommandCompilerC(
        "avrpath/", new ArrayList(Arrays.asList(new String[] {})),
        "main.c", "main.o"),
      new String[] {
        "avrpath/avr-gcc", "-c", "-g", "-Os", "-w", "-mmcu=testmcu",
        "-DF_CPU=testf_cpu", "main.c", "-omain.o" });
    compare(
      Compiler.getCommandCompilerC(
        "avrpath/",
        new ArrayList(Arrays.asList(new String[] { "inc1/", "inc2/" })),
        "main.c", "main.o"),
      new String[] {
        "avrpath/avr-gcc", "-c", "-g", "-Os", "-w", "-mmcu=testmcu",
        "-DF_CPU=testf_cpu", "-Iinc1/", "-Iinc2/", "main.c", "-omain.o" });
  }
  
  @Test public void testGetCommandCompilerCPP() {
    compare(
      Compiler.getCommandCompilerCPP(
        "avrpath/", new ArrayList(Arrays.asList(new String[] {})),
        "main.cpp", "main.o"),
      new String[] {
        "avrpath/avr-g++", "-c", "-g", "-Os", "-w", "-fno-exceptions",
        "-mmcu=testmcu", "-DF_CPU=testf_cpu", "main.cpp", "-omain.o" });
    compare(
      Compiler.getCommandCompilerCPP(
        "avrpath/",
        new ArrayList(Arrays.asList(new String[] { "inc1/", "inc2/" })),
        "main.cpp", "main.o"),
      new String[] {
        "avrpath/avr-g++", "-c", "-g", "-Os", "-w", "-fno-exceptions",
        "-mmcu=testmcu", "-DF_CPU=testf_cpu", "-Iinc1/", "-Iinc2/",
        "main.cpp", "-omain.o" });
  }
}
