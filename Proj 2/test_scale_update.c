#include "scale.h"

// macro to set up a test with given name, print the source of the
// test; very hacky, fragile, but useful
#define IF_TEST(TNAME) \
  if( RUNALL || strcmp( TNAME, test_name)==0 ) { \
    sprintf(sysbuf,"awk 'NR==(%d){P=1;gsub(\"^ *\",\"\");} P==1 && /ENDTEST/{P=0; print \"}\\n---OUTPUT---\"} P==1{print}' %s", __LINE__, __FILE__); \
    system(sysbuf); nrun++;  \
  } \
  if( RUNALL || strcmp( TNAME, test_name)==0 )

char sysbuf[1024];
int RUNALL = 0;
int nrun = 0;

void print_scale(scale_t *scale){
  printf("scale = {\n"); 
  printf("  weight     = %hd\n",scale->weight);
  printf("  mode       = %hhd\n",scale->mode);
  printf("  indicators = %s\n",bitstr(scale->indicators, &indicatorspec));
  printf("}\n");
}

void print_ports(){
  printf("SCALE_SENSOR_PORT: %hd\n", SCALE_SENSOR_PORT);
  printf("SCALE_TARE_PORT:   %hd\n", SCALE_TARE_PORT);
  printf("SCALE_STAUS_PORT:  %s\n",bitstr(SCALE_STATUS_PORT, &statspec));
  printf("SCALE_DISPLAY_PORT:\n");
  printf("bits:  %s\n",bitstr(SCALE_DISPLAY_PORT, &dispspec));
  printf("index: %s\n",bitstr_index(&dispspec));
}

// prints an integer as though it were the display
void print_dispint_display(int dispint){
  printf("DISPINT BITS ARE:\n");
  printf("bits:  %s\n",bitstr(dispint, &dispspec));
  printf("index: %s\n",bitstr_index(&dispspec));
  int old_port = SCALE_DISPLAY_PORT;
  printf("DISPINT AS DISPLAY:\n");
  SCALE_DISPLAY_PORT = dispint;
  print_display();
  SCALE_DISPLAY_PORT = old_port;
}

int main(int argc, char *argv[]){
  if(argc < 2){
    printf("usage: %s <test_name>\n", argv[0]);
    return 1;
  }
  char *test_name = argv[1];
  char sysbuf[1024];
  if(strcmp(test_name,"ALL")==0){
    RUNALL = 1;
  }  

  // malloc() these so Valgrind checks uninitialized data
  scale_t *scale = malloc(sizeof(scale_t)); // used for tests that set a scale
  int *dispint = malloc(sizeof(int));       // used for tests that set the display
  int ret;

  ////////////////////////////////////////////////////////////////////////////////
  // scale_from_ports() TESTS 10
  ////////////////////////////////////////////////////////////////////////////////

  IF_TEST("scale_from_ports_01") {
    // Basic test that the fields of scale are set properly with zeros
    // in all sensors. Ports should not be altered.
    SCALE_SENSOR_PORT  = 0;
    SCALE_TARE_PORT    = 0;
    SCALE_STATUS_PORT  = 0b00000000; // ounces unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  IF_TEST("scale_from_ports_02") {
    // Basic test that the fields of scale are set properly from a
    // non-zero sensor value.
    SCALE_SENSOR_PORT  = 562;
    SCALE_TARE_PORT    = 0;
    SCALE_STATUS_PORT  = 0b00000000; // ounces unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  IF_TEST("scale_from_ports_03") {
    // Basic test that the fields of scale are set properly from a
    // non-zero sensor value AND that value is converted to pounds
    // correctly. The indicators field should also show that the
    // pounds light rather than the ounces light should be on.
    SCALE_SENSOR_PORT  = 160*1 + 3;  // slightly more than 1 pound
    SCALE_TARE_PORT    = 0;
    SCALE_STATUS_PORT  = 0b00000100; // pounds unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST


  IF_TEST("scale_from_ports_04") {
    // Basic test that the fields of scale are set properly from a
    // non-zero sensor value AND that value is converted to pounds
    // correctly. In this case, pounds should be rounded up by 1.
    SCALE_SENSOR_PORT  = 160*1 + 10; // 170oz = 1.06 lbs -> 1.1 lbs
    SCALE_TARE_PORT    = 0;
    SCALE_STATUS_PORT  = 0b00000100; // pounds unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  IF_TEST("scale_from_ports_05") {
    // Checks that the stored Tare value is subtracted from the sensor
    // value in the weight that is set. The indicators for ounces AND
    // Tare non-zero should be set.
    SCALE_SENSOR_PORT  = 720;
    SCALE_TARE_PORT    = 250;
    SCALE_STATUS_PORT  = 0b00000000; // ounces unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  IF_TEST("scale_from_ports_06") {
    // Checks that the stored Tare value is subtracted from the sensor
    // value in the weight that is set. In this case, the resulting
    // weight is negative. Indictators for ounces and non-zero Tare is
    // stored.
    SCALE_SENSOR_PORT  = 256;
    SCALE_TARE_PORT    = 515;
    SCALE_STATUS_PORT  = 0b00000000; // ounces unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  IF_TEST("scale_from_ports_07") {
    // Checks that the stored Tare Button is pressed, the scale is set
    // to MODE_TARE with 0 for the remaining fields.
    SCALE_SENSOR_PORT  = 399;
    SCALE_TARE_PORT    = 727;
    SCALE_STATUS_PORT  = 0b00100000; // ounces unit, Tare button
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  IF_TEST("scale_from_ports_08") {
    // Checks that the extremal value for the sensor port is still
    // valid. This test should behave normally.
    SCALE_SENSOR_PORT  = 999;
    SCALE_TARE_PORT    = 0;
    SCALE_STATUS_PORT  = 0b00000100; // pounds unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  IF_TEST("scale_from_ports_09") {
    // Checks that out of an range value for the sensor will trigger
    // the error mode to be set.
    printf("Out of range on SCALE_SENSOR_PORT\n");
    SCALE_SENSOR_PORT  = 1200;
    SCALE_TARE_PORT    = 0;
    SCALE_STATUS_PORT  = 0b00000000; // ounces unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
    printf("Out of range on SCALE_TARE_PORT\n");
    SCALE_SENSOR_PORT  = 555;
    SCALE_TARE_PORT    = -27;
    SCALE_STATUS_PORT  = 0b00000100; // pounds unit
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  IF_TEST("scale_from_ports_10") {
    // Error conditions should e checked before the Tare button. This
    // test should cause a MODE_ERROR to be set due to the sensors
    // having out-of-range values, not Tare mode.
    printf("Error takes precedence over Tare button\n");
    SCALE_SENSOR_PORT  = -100;
    SCALE_TARE_PORT    = 2700;
    SCALE_STATUS_PORT  = 0b00100100; // pounds unit, tare button
    SCALE_DISPLAY_PORT = -1;
    ret = scale_from_ports(scale);
    printf("ret: %d\n",ret);
    print_scale( scale );
    print_ports();
  } // ENDTEST

  ////////////////////////////////////////////////////////////////////////////////
  // scale_display_special() TESTS 3
  ////////////////////////////////////////////////////////////////////////////////
  IF_TEST("scale_display_special_01") {
    // Check the results of displaying a MODE_TARE scale_t which
    // should create the display "STOR" to indicate the sensor value
    // is stored.  The ports should not change in any display
    // function. Rather, the provided `dispint` location should be set
    // to the bits that are to be displayed.
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =  270;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000000; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = 0,
      .indicators = 0,
      .mode       = MODE_TARE,
    };
    ret = scale_display_special(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_special_02") {
    // Check the results of displaying a MODE_ERROR scale_t which
    // should create the display "ERR" to indicate the a problem.  The
    // ports should not change in any display function. Rather, the
    // provided `dispint` location should be set to the bits that are
    // to be displayed.
    SCALE_SENSOR_PORT  = 0;          // all ports should be  
    SCALE_TARE_PORT    = 0;          // ignored in display 
    SCALE_STATUS_PORT  = 0b00000000; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = 0,
      .indicators = 0,
      .mode       = MODE_ERROR,
    };
    ret = scale_display_special(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_special_03") {
    // Check the results of calling with a MODE_SHOW scale do nothing
    // and return 1 without changing the display.
    SCALE_SENSOR_PORT  = 0;          // all ports should be  
    SCALE_TARE_PORT    = 0;          // ignored in display 
    SCALE_STATUS_PORT  = 0b00000000; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = 100,
      .indicators = 0,
      .mode       = MODE_SHOW,
    };
    *dispint = -1;
    ret = scale_display_special(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  ////////////////////////////////////////////////////////////////////////////////
  // scale_display_weight() TESTS 10
  // 512 9 76 3 84
  ////////////////////////////////////////////////////////////////////////////////

  IF_TEST("scale_display_weight_01") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. This test
    // has a positive weight with 3 digits and with the ounces
    // indicator on. Display functions should ignore all Ports and use
    // only the data in the provided scale_t.
    SCALE_SENSOR_PORT  =  100;       // all ports should be  
    SCALE_TARE_PORT    =  270;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000000; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = 123,
      .indicators = 0b0000001,  // ounces on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_02") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. This test
    // has a positive weight with 3 digits and with the ounces
    // indicator on. The displayed weight is at the maximum
    // allowable by the scale. Display functions should ignore all
    // Ports and use only the data in the provided scale_t.
    SCALE_SENSOR_PORT  =  100;       // all ports should be  
    SCALE_TARE_PORT    =  270;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000000; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = 999,
      .indicators = 0b0000001,  // ounces on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_03") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. This test
    // has a positive weight with ONLY 2 DIGITS but there should be no
    // leading 0 in this case, just a blank space. 
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =    0;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000100; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = 45,
      .indicators = 0b0000001,  // ounces on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_04") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. This test
    // has a positive weight with ONLY 1 DIGIT which should have a 0
    // preceding the decimal place but a blank in the next place.
    //
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =    0;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000100; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = 6,
      .indicators = 0b0000001,  // ounces on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_05") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. The
    // weight is negative with 3 digits so that the leftmost position
    // should have the negative sign in it.
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =    0;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000100; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = -781,
      .indicators = 0b0000001,  // ounces on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_06") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. The
    // weight is negative with 2 digits so that the leftmost position
    // is blank and the 2nd digit is has the negative sign in it.
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =    0;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000100; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = -59,
      .indicators = 0b0000001,  // ounces on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_07") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. The
    // weight is negative with 1 digit so that a leading zero is
    // present preceded by the negative sign.
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =    0;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000100; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = -7,
      .indicators = 0b0000001,  // ounces on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_08") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. This test
    // turns on the pound and tare indicators.
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =    0;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000000; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = 59,
      .indicators = 0b0000110,  // pounds/tare on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_09") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. This test
    // turns on the pound and tare indicators and has a negative
    // weight.
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =    0;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000000; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = -54,
      .indicators = 0b0000110,  // pounds/tare on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  IF_TEST("scale_display_weight_10") {
    // Check the results of displaying a MODE_SHOW scale_t sets the
    // display according to the weight and indicator fields. This test
    // turns on the ounces and tare indicators and has a negative
    // weight at the maximum value allowed.
    SCALE_SENSOR_PORT  = -100;       // all ports should be  
    SCALE_TARE_PORT    =    0;       // ignored in display 
    SCALE_STATUS_PORT  = 0b00000000; // functions
    SCALE_DISPLAY_PORT = -1;
    scale_t scale = {
      .weight     = -999,
      .indicators = 0b0000101,  // ounces/tare on
      .mode       = MODE_SHOW,
    };
    ret = scale_display_weight(scale, dispint);
    printf("ret: %d\n",ret);
    print_ports();
    print_dispint_display(*dispint);
  } // ENDTEST

  ////////////////////////////////////////////////////////////////////////////////
  // scale_update() TESTS 8
  ////////////////////////////////////////////////////////////////////////////////

  IF_TEST("scale_update_01") {
    // Check scale_update() which should call other functions to
    // determine the weight to display from Ports and set the display
    // accordingly. This is a normal positive weight of 3 digits in
    // ounces. The Tare value is 0 so no indicator is shown for it.
    SCALE_SENSOR_PORT  = 321;
    SCALE_TARE_PORT    = 0;
    SCALE_STATUS_PORT  = 0b00000000;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  IF_TEST("scale_update_02") {
    // Check scale_update() which should call other functions to
    // determine the weight to display from Ports and set the display
    // accordingly. This is a normal positive weight of 3 digits in
    // ounces. The Tare value is non-zero so the -T indicator should
    // be turned on.
    SCALE_SENSOR_PORT  = 321;
    SCALE_TARE_PORT    =  45;
    SCALE_STATUS_PORT  = 0b00000000;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  IF_TEST("scale_update_03") {
    // Check scale_update() which should call other functions to
    // determine the weight to display from Ports and set the display
    // accordingly. This is a normal negative weight of 3 digits in
    // ounces. The Tare value is non-zero so the -T indicator should
    // be turned on.
    SCALE_SENSOR_PORT  =  15;
    SCALE_TARE_PORT    = 795;
    SCALE_STATUS_PORT  = 0b00000000;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  IF_TEST("scale_update_04") {
    // Check scale_update() which should call other functions to
    // determine the weight to display from Ports and set the display
    // accordingly. This is a normal positive weight of 2 digits in
    // pounds with no Tare value. The weight in pounds should be
    // rounded up.
    SCALE_SENSOR_PORT  = 160*4 + 5*16 + 12; // 4.575 lbs -> 4.6
    SCALE_TARE_PORT    = 0;
    SCALE_STATUS_PORT  = 0b00000100;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  IF_TEST("scale_update_05") {
    // Check scale_update() which should call other functions to
    // determine the weight to display from Ports and set the display
    // accordingly. This is a normal positive weight of 2 digits in
    // pounds with a 0 Sensor and positive Tare value leading to a
    // negative weight. The weight in pounds should be rounded down
    SCALE_SENSOR_PORT  = 0;
    SCALE_TARE_PORT    = 160*2 + 9*16 + 6; // 2.9375 lbs -> 2.90
    SCALE_STATUS_PORT  = 0b00000100;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  IF_TEST("scale_update_06") {
    // Check scale_update() which should detect that a MODE_TARE and
    // copy the value from SCALE_SENSOR_PORT to SCALE_TARE_PORT along
    // with setting the display to "STOR".
    SCALE_SENSOR_PORT  = 250;
    SCALE_TARE_PORT    = 0;     // should change to 250
    SCALE_STATUS_PORT  = 0b00100000;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  IF_TEST("scale_update_07") {
    // Check scale_update() detects that a negative sensor value
    // should lead to MODE_ERROR and should display "ERR".
    SCALE_SENSOR_PORT  = -16;
    SCALE_TARE_PORT    = 0; 
    SCALE_STATUS_PORT  = 0b00000100;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  IF_TEST("scale_update_08") {
    // Check scale_update() detects that a too-big sensor value
    // should lead to MODE_ERROR and should display "ERR".
    SCALE_SENSOR_PORT  = 1005;
    SCALE_TARE_PORT    = 0; 
    SCALE_STATUS_PORT  = 0b00000000;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  IF_TEST("scale_update_09") {
    // Check scale_update() detects that a Tare value that is too
    // large should lead to MODE_ERROR and should display "ERR". In
    // this case the Tare button is pressed but the error condition
    // takes precedence.
    SCALE_SENSOR_PORT  =  235;
    SCALE_TARE_PORT    = 4351; 
    SCALE_STATUS_PORT  = 0b00100000;
    SCALE_DISPLAY_PORT = -1;
    ret = scale_update();
    printf("ret: %d\n",ret);
    print_ports();
    printf("Scale Display:\n");
    print_display();
  } // ENDTEST

  ////////////////////////////////////////////////////////////////////////////////
  // END MATTER
  ////////////////////////////////////////////////////////////////////////////////
  if(nrun == 0){
    printf("No test named '%s' found\n",test_name);
    return 1;
  }
  else if(nrun > 1){
    printf("%d tests run\n",nrun);
  }

  free(scale);
  free(dispint);

  return 0;
}
