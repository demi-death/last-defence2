import java.net.Socket;
import java.net.InetSocketAddress;
import java.io.IOException;
import processing.net.*;
String hostName;
Client client;

PFont largeFont;

int portNum = 1024;
int timeOut = 10000;

void settings() {
  size(800, 800);
}

void setup() {
  largeFont = createFont("Ubuntu-Regular.ttf", 50);
  background(0);
  textAlign(CENTER);
  textFont(largeFont);
  
  if (args == null || args.length < 2)
    hostName = "localhost";
  else
    hostName = args[1];
  String connectingTo = "Connecting to " + hostName + "...";
  println(connectingTo);
  text(connectingTo, width*0.5, height*0.5);
  try {
    Socket socket = new Socket();
    socket.connect(new InetSocketAddress(hostName, portNum), timeOut);
    client = new Client(this, socket);
    println("Connection success to " + hostName);
  } catch (Exception e) {
    println("Connection Failure: " + e.getMessage());
    exit();
  }
}
JSONObject receivedJSON;
final Object lockJSON = new Object();
void draw() {
  background(0,200,0);
  if(!client.active())
  {
    textFont(largeFont);
    text("Connection failed. Reconnecting to the server.", width*0.5, height*0.5);
  }
  synchronized(lockJSON) {
  }
  switch(receivedJSON.getInt("status"))
  {
  case 0:
    receivedJSON.
  case 1:
  case 2:
  }
}

void startReceiver() {
  class ReadJson implements Runnable{
    private StringBuilder buffer = new StringBuilder();
    public void run(){
      for(;;) {
        if(client.available() == 0)
          continue;
        String chunk = client.readString();
        if (chunk == null)
          continue;
        int lastDelim = -1, pLastDelim = -1;
        for(;;){
          int idx = chunk.indexOf('\n', lastDelim + 1);
          if(idx == -1)
            break;
          pLastDelim = lastDelim;
          lastDelim = idx;
        }
        if(pLastDelim == -1)
        {
          if(lastDelim == -1) // if buffer is "..."
            buffer.append(chunk);
          else // if buffer is "...}\n{..."
          {
            buffer.append(chunk, 0, lastDelim);
            JSONObject tempJSON = parseJSONObject(buffer.toString());
            synchronized(lockJSON){
              receivedJSON = tempJSON;
            }
            buffer.setLength(0);
            buffer.append(chunk, lastDelim + 1, chunk.length());
          }
        }
        else // if buffer is "...}\n{...}\n{..."
        {
          JSONObject tempJSON = parseJSONObject(chunk.substring(pLastDelim + 1, lastDelim));
          synchronized(lockJSON){
            receivedJSON = tempJSON;
          }
          buffer.setLength(0);
          buffer.append(chunk, lastDelim + 1, chunk.length());
        }
        try {
          Thread.sleep(1);
        } catch (InterruptedException e) {
          println("Receiver thread interrupted");
          break;
        }
      }
    }
  };
  Thread thread = new Thread(new ReadJson());
  thread.start();
}
