import java.net.Socket;
import java.net.InetSocketAddress;
import java.io.IOException;
import processing.net.*;
String hostName;
Client client;

PFont largeFont;

int portNum = 1024;
int timeOut = 10000;

String receivedData;
final Object receivedDataLock = new Object();
class ReadRecentData implements Runnable{
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
          synchronized(receivedDataLock){
            receivedData = buffer.toString();
          }
          buffer.setLength(0);
          buffer.append(chunk, lastDelim + 1, chunk.length());
        }
      }
      else // if buffer is "...}\n{...}\n{..."
      {
        synchronized(receivedDataLock){
          receivedData = chunk.substring(pLastDelim + 1, lastDelim);
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
Thread readRecentData = new Thread(new ReadRecentData());
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
    readRecentData.start();
  } catch (Exception e) {
    println("Connection Failure: " + e.getMessage());
    exit();
  }
}
JSONObject json;
JSONObject myStat;
JSONObject crystalEnt;
JSONObject gameStat;
JSONArray entities;
JSONArray bullets;
JSONArray dropItems;
void draw() {
  background(0,200,0);
  pushMatrix();
  if(!client.active())
  {
    textFont(largeFont);
    text("Connection failed. Reconnecting to the server.", width*0.5, height*0.5);
  }
  else
  {
    synchronized(receivedDataLock) {
      json = parseJSONObject(receivedData);
    }
    if(json != null)
    {
      myStat = json.getJSONObject("myStat");
      crystalEnt = json.getJSONObject("crystalStat");
      gameStat = json.getJSONObject("gameStat");
      entities = json.getJSONArray("entities");
      bullets = json.getJSONArray("bullets");
      dropItems = json.getJSONArray("dropItems");
    }
  }
  int zombRemain = gameStat.getInt("zombRemain");
  
  int ammo = myStat.getInt("ammo");
  int ammoMax = myStat.getInt("ammoMax");
  float reloadingTime = myStat.getFloat("reloadingTime");
  float reloadingRemain = myStat.getFloat("reloadingRemain");
  JSONObject myEnt = myStat.getJSONObject("ent");
  
  float myx = myEnt.getInt("x");
  float myy = myEnt.getInt("y");
  int myHP = myEnt.getInt("hp");
  int myHPMax = myEnt.getInt("hpMax");
  
  int crystalHP = crystalEnt.getInt("hp");
  int crystalHPMax = crystalEnt.getInt("hpMax");
  
  translate(width/2, height/2);
  for(int i = 0; i < entities.size(); ++i)
  {
    JSONObject entity = entities.getJSONObject(i);
    int team = entity.getInt("team");
    float x = entity.getFloat("x");
    float y = entity.getFloat("y");
    float size = entity.getFloat("size");
    int hp = myStat.getInt("hp");
    int hpMax = myStat.getInt("hpMax");
    
    if(team == 1)
      fill(17, 74, 7);
    else
      fill(255,255,255);
    ellipse(x - myx, y - myy, size, size);
    fill(0);
    rect(x - myx - size * 0.75, y - myy - size * 0.7, size * 1.5, size * 0.2);
    if(team == 1)
      fill(255, 0, 0);
    else
      fill(0,255,0);
    rect(x - myx - size * 0.75, y - myy - size * 0.7, size * 1.5 * (hp / hpMax), size * 0.2);
  }
  popMatrix();
}
