package processing.app.preproc.test;

import java.io.Reader;
import java.io.File;
import java.io.FileReader;

import java.util.List;
import java.util.ArrayList;

import org.junit.Test;
import org.junit.Before;
import org.junit.Assert;
import static org.junit.Assert.*;

import com.oroinc.text.regex.*;

import processing.app.preproc.PdePreprocessor;

public class PdePreprocessorTest {
  private String[][] stripTests = new String[][] { {
    "multi-line comments",
    "a c g",
    "a/*b*/c/*d\n" + 
    "e\n" +
    "f*/g"
  }, {
    "single-line comments",
    "a \n" +
    "c\n" +
    " ",
    "a//b\n" +
    "c\n" +
    "//d"
  }, {
    "double-quoted strings",
    "a c f",
    "a\"b\"c\"d\\\"e\\\\\"f"
  }, {
    "single-quoted strings",
    "a c",
    "a'b'c"
  }, {
    "single- and multi-line comments",
    "a \n" +
    "d\n" +
    "e ",
    "a//b/*c\n" +
    "d\n" +
    "e//f*/g"
  }, {
    "single- and double-quoted strings",
    "a b c",
    "a'\"'b'\"'c"
  }, {
    "strings in comments",
    "a \n" +
    "d\n" +
    "e ",
    "a//b\"c\n" +
    "d\n" +
    "e//f\"g"
  }, {
    "comments in strings",
    "a e",
    "a\"b//c\"e"
  } };

  @Test public void testStrip() throws Exception {
    for (int i = 0; i < stripTests.length; i++) {
      assertEquals(stripTests[i][0], stripTests[i][1],
        new PdePreprocessor().strip(stripTests[i][2]));
    }
  };
}