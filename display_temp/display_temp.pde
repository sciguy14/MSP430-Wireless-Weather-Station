//Jeremy Blum Processing Display

import processing.serial.*;
Serial port;
int light = 0;
String light_display = "";
int humidity = 0;
String humidity_display = "";
int temp = 0;
String tempC_display = "";
String data = "";

PImage img;
PFont font;

int[] light_array = new int[400];
int[] humidity_array = new int[400];
int[] temp_array = new int[400];

void setup(){
  size(1024, 768);
  port = new Serial(this, "COM9", 9600);
  port.bufferUntil('.');
  smooth();
  img = loadImage("display.jpg");
  font = loadFont("PalatinoLinotype-Bold-40.vlw");
  textFont(font);
}

void draw(){
  image(img, 0, 0);
  fill(60, 35, 0);
  textSize(40);
  textAlign(LEFT);
  text("Cornell Weather Report", 480, 120);
  fill(255,0,0);
  text("Light:", 520, 200);
  fill(0,255,0);
  text("Humidity:", 520, 250);
  fill(0,0,255);
  text("Temperature:", 520, 300);
  fill(0);
  text(light_display, 780, 200);
  text(humidity_display, 780, 250);
  text(tempC_display, 780, 300);
  textSize(16);
  text("Jeremy Blum and Matt Newberg - ECE 3140", 530, 370 );
  
  
  //Grid Lines
  for(int i = 0 ;i<=width/20;i++)
  {
    strokeWeight(1);
    stroke(200);
    line((-frameCount%20)+i*20,0,(-frameCount%20)+i*20,height);
 
    line(0,i*20,width,i*20);
  }
  
  //LIGHT GRAPH
  noFill();
  stroke(255,0,0);
  strokeWeight(5);
  beginShape();
  for(int i = 0; i<light_array.length;i++)
  {
    vertex(i,350-light_array[i]);
  }
  endShape();
  for(int i = 1; i<light_array.length;i++)
  {
    light_array[i-1] = light_array[i];
  }
  light_array[light_array.length-1]=(int) ((light/100.0)*768.0 - 500.0);
  
  
  //HUMIDITY GRAPH
  noFill();
  stroke(0,255,0);
  strokeWeight(5);
  beginShape();
  for(int i = 0; i<humidity_array.length;i++)
  {
    vertex(i,350-humidity_array[i]);
  }
  endShape();
  for(int i = 1; i<humidity_array.length;i++)
  {
    humidity_array[i-1] = humidity_array[i];
  }
  humidity_array[humidity_array.length-1]=(int) ((humidity/100.0)*768.0 - 450.0);
  
  //TEMP GRAPH
  noFill();
  stroke(0,0,255);
  strokeWeight(5);
  beginShape();
  for(int i = 0; i<temp_array.length;i++)
  {
    vertex(i,350-temp_array[i]);
  }
  endShape();
  for(int i = 1; i<temp_array.length;i++)
  {
    temp_array[i-1] = temp_array[i];
  }
  temp_array[temp_array.length-1]=(int) (((temp-273)/40.0)*768.0 - 450.0);
  
  
}

void serialEvent(Serial port){
  data = port.readStringUntil('.');
  println(data);
  data = data.substring(0, data.length() - 1);
  // Fetch the light level
  light = 100*(data.charAt(0)-'0') + 10*(data.charAt(1)-'0') + (data.charAt(2)-'0');
  light_display = Integer.toString(light) + "%";
  // Fetch the humidity
  humidity = 100*(data.charAt(3)-'0') + 10*(data.charAt(4)-'0') + (data.charAt(5)-'0');
  humidity_display = Integer.toString(humidity) + "%";
  //Fetch the temperature
  temp = 100*(data.charAt(6)-'0') + 10*(data.charAt(7)-'0') + (data.charAt(8)-'0');
  tempC_display = Integer.toString(temp - 273) + "C";
}
