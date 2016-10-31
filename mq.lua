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

function change_led (led, target, ft)
  t = target
  c = lc[led]

  diff=target-c
  step = diff / (ft/led_update_time)

  tmr.alarm(led, led_update_time, tmr.ALARM_AUTO, function() 
    if (step > 0 and lc[led] + step < target) or (step < 0 and lc[led] + step > target) then
      lc[led] = lc[led] + step;
    else 
      lc[led] = target
      tmr.unregister(led)
    end
    pwm.setduty(led+2, lc[led])
        
  end)
end
-- on publish message receive event
m:on("message", function(conn, topic, data)
  print(topic) 
  if data ~= nil then
    print(data)
    print(node.heap())

    payload = cjson.decode(data)['payload']
 
    if topic == topic1 then
      t=tonumber(payload['l'])
      ft=tonumber(payload['ft'])
      change_led(1,t,ft)
    elseif topic == topic2 then
      t=tonumber(payload['l'])
      ft=tonumber(payload['ft'])
      change_led(2,t,ft)
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
