topic = "/kbdim/".. wifi.sta.getmac()
topic1 = topic .. "/1/"
topic2 = topic .. "/2/"
topicu = topic .. "/u/"
topicr = topic .. "/ur/"
ok = [[{"c":"ok"}]]
broker_addr = "mqtt.lan"
broker_port = 1883 

pwm.setup(3, 100, 001)
pwm.setup(4, 100, 001)
pwm.start(3)
pwm.start(4)

led_update_time = 20
lc = {}
lc[1] = 0
lc[2] = 0

upload_name = ""
upload_line = 0

m = mqtt.Client(wifi.sta.getmac(), 120)

function wait_for_wifi()
  tmr.alarm(0, 1000, tmr.ALARM_AUTO, function()
    if wifi.sta.status() == 5 then
	    tmr.unregister(0)
      connect_channels()
    end
  end)
end

function connect_channels ()
  tmr.alarm(0, 1000, tmr.ALARM_AUTO, function()
  m:connect(broker_addr, broker_port, 0, function(conn)
    tmr.unregister(0)
    m:subscribe(topic1 ,0, function(conn)
      m:publish(topic1.."status/","leddim active",0,0, function(conn)
        m:subscribe(topic2 ,0, function(conn)
          m:publish(topic2.."status/","leddim active",0,0, function(conn)
            m:subscribe(topicu ,0, function(conn)
              print("ch ok")
            end)
          end)
        end)
      end)
    end)
  end)
  end)
end

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

m:lwt("/lwt", wifi.sta.getmac(), 0, 0)
m:on("offline", function(con)
     wait_for_wifi()
end)


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
    elseif topic == topicu then
      if payload['command'] == 'open' then
            
        print("open file")
        upload_line = 0
        upload_name = payload['file']
        file.close()
        file.remove("tmp.lua")
        file.open("tmp.lua", "w+")
        m:publish(topicr,ok,0,0)
      elseif payload['command'] == 'close' then
        print("close file")
        
        file.close()
        file.remove(upload_name)
        file.rename("tmp.lua", upload_name)
        node.compile(upload_name)
        m:publish(topicr,ok,0,0)
    
      elseif payload['command'] == 'l' then
        print("write line")
        
        upload_line = upload_line + 1
        file.writeline(payload["line"])
        m:publish(topicr,ok,0,0)
      elseif payload['command'] == 'run' then
        print ("running file")
        dofile(payload['file'])
      end
      



    end

  end
end)


wait_for_wifi()
