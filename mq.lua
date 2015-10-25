topic = "/leddim/".. wifi.sta.getmac() .. "/"
broker_addr = "mqtt_addr"
broker_port = 1883 

pwm.setup(3, 100, 001)
pwm.setup(4, 100, 001)
pwm.start(3)
pwm.start(4)

led1_current=005
led1_target=005
led1_fadetimer = 0
led1_diff = 0
led1_step = 0
led1_update_time = 20

led2_current=005
led2_target=005
led2_fadetimer = 0
led2_diff = 0
led2_step = 0
led2_update_time = 20

m = mqtt.Client(wifi.sta.getmac(), 120)
m:lwt("/lwt", wifi.sta.getmac(), 0, 0)
m:on("offline", function(con)
     print ("reconnecting...")
     print(node.heap())
     tmr.alarm(1, 10000, 0, function()
       m:connect(broker_addr, broker_port, 0, function(conn)
          print("reconnected")
          m:subscribe(topic ,0, function(conn)
               print("subbed to " .. topic)
               m:publish(topic.."status/","leddim reconnected",0,0, function(conn) print("sent sub") end)
          end)
       end)
     end)
end)

-- on publish message receive event
m:on("message", function(conn, topic, data)
  print(topic .. ":" )
  if data ~= nil then
    print(data)
    payload = cjson.decode(data)

    print("Input:"..data) 
 
    if payload['led'] == 1 then
      led1_target=tonumber(payload['l'])
      print("Received LED1 Target Value: "..led1_target)

      led1_fadetimer=tonumber(payload['ft'])

      print("Received LED1 Target Value: "..led1_target)
      print("Received LED1 Current Value: " .. led1_current)

      print ("Received LED1 Fadetimer: " ..led1_fadetimer)
      led1_diff=(led1_target)-(led1_current)
     
      -- Get steps / second (on 10Hz).
      led1_step = led1_diff / (led1_fadetimer/led1_update_time);
    
      
      print("Step value: "..led1_step);

      tmr.alarm(1, led1_update_time, 1, function() 
        
        if led1_step > 0 and led1_current + led1_step < led1_target then
           led1_current = led1_current + led1_step;
        elseif led1_step < 0 and led1_current + led1_step > led1_target then
          led1_current = led1_current + led1_step;
        else 
          led1_current = led1_target;
          
          tmr.stop(1)
        end;
        
        pwm.setduty(3, led1_current)
        
      end )
    end

    if payload['led'] == 2 then
    
      led2_target=tonumber(payload['l'])
      print("Received LED2 Target Value: "..led2_target)

      led2_fadetimer=tonumber(payload['ft'])

      print("Received LED2 Target Value: "..led2_target)
      print("Received LED2 Current Value: " .. led2_current)

      print ("Received LED2 Fadetimer: " ..led2_fadetimer)
      led2_diff=(led2_target)-(led2_current)
     
      -- Get steps / second (on 10Hz).
      led2_step = led2_diff / (led2_fadetimer/led2_update_time);
    
      
      print("Step value: "..led2_step);


      tmr.alarm(2, led2_update_time, 1, function() 
        
        if led2_step > 0 and led2_current + led2_step < led2_target then
           led2_current = led2_current + led2_step;
        elseif led2_step < 0 and led2_current + led2_step > led2_target then
          led2_current = led2_current + led2_step;
        else 
          led2_current = led2_target;
          tmr.stop(2)
        end;
        
        pwm.setduty(4, led2_current)
        
      end )
    end
	
  end
end)

tmr.alarm(0, 1000, 1, function()
 if wifi.sta.status() == 5 then
     tmr.stop(0)
     m:connect(broker_addr, broker_port, 0, function(conn)
          print("connected")
          m:subscribe(topic ,0, function(conn)
               print("subbed to " .. topic)
               m:publish(topic.."status/","leddim active",0,0, function(conn) print("sent sub") end)
          end)
     end)
 end
end)
