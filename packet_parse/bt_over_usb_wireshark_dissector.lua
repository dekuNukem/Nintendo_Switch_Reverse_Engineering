-- CTCaer's Joy-Con BT over USB protocol dissect script for Wireshark
-- Version 0.9
-- This is provided as is. No license. :)
p_jc_btusb = Proto ("jc_btusb","Joy-Con BT over USB")
local f_src = ProtoField.uint8("jc_btusb.src", "SOURCE BT Device", base.HEX)
local f_dst = ProtoField.uint8("jc_btusb.dst", "DESTINATION BT Device", base.HEX)
local f_command = ProtoField.uint8("jc_btusb.cmd", "Command", base.HEX)
local f_inputid = ProtoField.uint8("jc_btusb.inputid", "id", base.HEX)
local f_lol = ProtoField.string("jc_btusb.lol", "test", base.HEX)
local f_subcommand = ProtoField.uint8("jc_btusb.sub", "Subcommand", base.HEX)
local f_timming = ProtoField.uint8("jc_btusb.time", "Timming byte", base.HEX)
local f_subcommandjoy = ProtoField.uint8("jc_btusb.sub2", "Sub cmd ack", base.HEX)
local f_rumble = ProtoField.uint64("jc_btusb.rumble", "Vibration pattern", base.HEX)

p_jc_btusb.fields = {f_lol, f_src, f_dst, f_command, f_inputid, f_subcommand, f_subcommandjoy, f_timming, f_rumble}

function p_jc_btusb.dissector (buf, pkt, root)
  --Check for no bt packet or JC/Host reply
  if buf:len() == 0 then return end
  local jc_host_check = buf(8,1):uint() == 0xa1 or buf(8,1):uint() == 0xa2
  if not jc_host_check then return end
  pkt.cols.protocol = p_jc_btusb.name
 
  subtree = root:add(p_jc_btusb, buf(0))
  --Joy-Con packet
  if buf(1,1):uint() == 0x20
    then 
      subtree:add(f_src, buf(0,1))
      subtree:add(f_inputid, buf(9,1))
      subtree:add(f_timming, buf(10,1))

      --check if it's a 0x21 input report
      if buf(9,1):uint() == 0x21 then
        --If reply type byte is > 0x80, then it's an ACK
        local switch_reply_type = buf(22,1):uint()
        local switch_ack_subcmd = buf(23,1):uint()

        if switch_reply_type == 0x82 then
          pkt.cols.info = "Device info"
          if buf(26,1):uint() == 0x01 then
            subtree:add(f_subcommandjoy, buf(22,1), buf(22,1):uint(), string.format("Subcmd reply: 0x02 [Device info], Left Joy-Con, FW Version: %X.%02x, MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", buf(24,1):uint(), buf(25,1):uint(), buf(28,1):uint(), buf(29,1):uint(), buf(30,1):uint(), buf(31,1):uint(), buf(32,1):uint(), buf(33,1):uint()))
          elseif buf(26,1):uint() == 0x02 then
            subtree:add(f_subcommandjoy, buf(22,1), buf(22,1):uint(), string.format("Subcmd reply: 0x02 [Device info], Right Joy-Con, FW Version: %X.%02x, MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", buf(24,1):uint(), buf(25,1):uint(), buf(28,1):uint(), buf(29,1):uint(), buf(30,1):uint(), buf(31,1):uint(), buf(32,1):uint(), buf(33,1):uint()))
          elseif buf(26,1):uint() == 0x03 then
            subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x02 [Device info], Pro Controller, FW Version: %X.%02x, MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", buf(24,1):uint(), buf(25,1):uint(), buf(28,1):uint(), buf(29,1):uint(), buf(30,1):uint(), buf(31,1):uint(), buf(32,1):uint(), buf(33,1):uint()))
          end
        elseif switch_reply_type == 0x90 then
          pkt.cols.info = "SPI Read data reply"
          subtree:add(f_subcommandjoy, buf(22,1), buf(22,1):uint(), "Subcmd reply: 0x10 [SPI Read data]")
        elseif switch_reply_type == 0x80 and switch_ack_subcmd == 0x08 then
          pkt.cols.info = "Shipment set ACK"
          subtree:add(f_subcommandjoy, buf(22,2), buf(22,2):uint(), "Subcmd reply: 0x08 [Shipment set ACK]")
        elseif switch_reply_type == 0x80 and switch_ack_subcmd == 0x03 then
          pkt.cols.info = "Input Report Format set ACK"
          subtree:add(f_subcommandjoy, buf(22,2), buf(22,2):uint(), "Subcmd reply: 0x03 [Input Report Format set ACK]")
        elseif switch_reply_type == 0x80 and switch_ack_subcmd == 0x11 then
          pkt.cols.info = "SPI Write OK"
          subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), "Subcmd reply: 0x11 [SPI Write ACK]")
        elseif switch_reply_type == 0x80 and switch_ack_subcmd == 0x05 then
          pkt.cols.info = "Page Info"
          subtree:add(f_subcommandjoy, buf(22,2), buf(22,2):uint(), string.format("Subcmd reply: 0x05 [Page Info], Arg: 0x%02x", buf(24,1):uint()))
        elseif switch_reply_type == 0x80 and switch_ack_subcmd == 0x22 then
          pkt.cols.info = "MCU Resume ACK"
          subtree:add(f_subcommandjoy, buf(22,2), buf(22,2):uint(), "Subcmd reply: 0x22 [MCU Resume ACK]")
        elseif switch_reply_type == 0x80 and switch_ack_subcmd == 0x40 then
          pkt.cols.info = "6-Axis Enable set ACK"
          subtree:add(f_subcommandjoy, buf(22,2), buf(22,2):uint(), "Subcmd reply: 0x40 [6-Axis Enable set ACK]")
        elseif switch_reply_type == 0x80 and switch_ack_subcmd == 0x48 then
          pkt.cols.info = "Vibration Enable set ACK"
          subtree:add(f_subcommandjoy, buf(22,2), buf(22,2):uint(), "Subcmd reply: 0x48 [Vibration Enable set ACK]")
        elseif switch_reply_type == 0x80 and switch_ack_subcmd == 0x30 then
          pkt.cols.info = "Player Lights set ACK"
          subtree:add(f_subcommandjoy, buf(22,2), buf(22,2):uint(), "Subcmd reply: 0x30 [Player Lights set ACK]")
        elseif switch_reply_type == 0x81 then
          pkt.cols.info = "Pairing IN"
          subtree:add(f_subcommandjoy, buf(22,1), buf(22,1):uint(), string.format("Subcmd reply: 0x01 [Pairing IN], Arg: %08x %08x %08x %08x %08x %08x %08x %08x %04x", buf(24,4):uint(), buf(28,4):uint(), buf(32,4):uint(), buf(36,4):uint(), buf(40,4):uint(), buf(44,4):uint(), buf(48,4):uint(), buf(52,4):uint(), buf(56,2):uint()))
        elseif switch_reply_type == 0xA0 then
          pkt.cols.info = "MCU Config data reply"
          subtree:add(f_subcommandjoy, buf(22,1), buf(22,1):uint(), string.format("Subcmd reply: 0x21 [MCU Config data], Arg: %08x %08x %08x %08x %08x %08x %08x %08x %04x", buf(24,4):uint(), buf(28,4):uint(), buf(32,4):uint(), buf(36,4):uint(), buf(40,4):uint(), buf(44,4):uint(), buf(48,4):uint(), buf(52,4):uint(), buf(56,2):uint()))
        elseif switch_reply_type == 0xA8 then
          pkt.cols.info = "Attachment data "
          subtree:add(f_subcommandjoy, buf(22,1), buf(22,1):uint(), string.format("Subcmd reply: 0x28? [Attachment data], Arg: %08x %08x %08x %08x %08x %08x %08x %08x %04x", buf(24,4):uint(), buf(28,4):uint(), buf(32,4):uint(), buf(36,4):uint(), buf(40,4):uint(), buf(44,4):uint(), buf(48,4):uint(), buf(52,4):uint(), buf(56,2):uint()))
        elseif switch_reply_type == 0xc0 then
          pkt.cols.info = "Sensor data"
          subtree:add(f_subcommandjoy, buf(22,1), buf(22,1):uint(), string.format("Subcmd reply: 0x41 [Sensor data], Arg: %08x %08x %08x %08x %08x %08x %08x %08x %04x", buf(24,4):uint(), buf(28,4):uint(), buf(32,4):uint(), buf(36,4):uint(), buf(40,4):uint(), buf(44,4):uint(), buf(48,4):uint(), buf(52,4):uint(), buf(56,2):uint()))
        elseif switch_reply_type == 0x83 then
          pkt.cols.info = "L\\R Elapsed time"
          local time1 = 10 * bit.bor(bit.lshift(buf(25,1):uint(), 8), buf(24,1):uint())
          local time2 = 10 * bit.bor(bit.lshift(buf(27,1):uint(), 8), buf(26,1):uint())
          local time3 = 10 * bit.bor(bit.lshift(buf(29,1):uint(), 8), buf(28,1):uint())
          local time4 = 10 * bit.bor(bit.lshift(buf(31,1):uint(), 8), buf(30,1):uint())
          local time5 = 10 * bit.bor(bit.lshift(buf(33,1):uint(), 8), buf(32,1):uint())
          local time6 = 10 * bit.bor(bit.lshift(buf(35,1):uint(), 8), buf(34,1):uint())
          local time7 = 10 * bit.bor(bit.lshift(buf(37,1):uint(), 8), buf(36,1):uint())
          subtree:add(f_subcommandjoy, buf(22,1), buf(22,1):uint(), string.format("Subcmd reply: 0x04 [L\\R Elapsed time], L: %dms, R: %dms, ZL: %dms, ZR: %dms, SL: %dms, SR: %dms, HOME: %dms", time1, time2, time3, time4, time5, time6, time7))
        elseif switch_reply_type > 0x7F then 
          subtree:add(f_subcommandjoy,buf(22,2))
        end

      elseif buf(9,1):uint() == 0x3f then
        pkt.cols.info = "Button state changed"
      end

      pkt.src = Address.ip(string.format("%02x.0.0.0", buf(0,1):uint()))
      
  --Host packet
  elseif buf(1,1):uint() == 0x00
    then
      subtree:add(f_dst, buf(0,1))
      subtree:add(f_command, buf(9,1))
      subtree:add(f_timming, buf(10,1))

      --Check for vibration command only
      if buf(9,1):uint() == 0x10 then 
        pkt.cols.info = "Vibration only set"
        --Calculate real vibration values for left LRA
        local lf_l = buf(13,1):uint()
        local la_l = buf(14,1):uint()
        if lf_l > 127 then
          lf_l = lf_l - 0x80
          la_l = la_l + 0x8000
        end
        local hf_l = buf(11,1):uint()
        local ha_l = buf(12,1):uint()
        local ha_lsb = bit.band(ha_l, -ha_l)
        if ha_lsb == 0x01 then
          hf_l = hf_l + 0x0100
          ha_l = ha_l - 0x01
        end
        --Calculate real vibration values for right LRA
        local lf_r = buf(17,1):uint()
        local la_r = buf(18,1):uint()
        if lf_r > 127 then
          lf_r = lf_r - 0x80
          la_r = la_r + 0x8000
        end
        local hf_r = buf(15,1):uint()
        local ha_r = buf(16,1):uint()
        ha_lsb = bit.band(ha_l, -ha_l)
        if ha_lsb == 0x01 then
          hf_r = hf_r + 0x0100
          ha_r = ha_r - 0x01
        end
        subtree:add(f_rumble, buf(11,8), buf(11,8):uint64(), string.format("Vibration, Left: HF: %04x HA: %02x LF: %02x LA: %04x, Right: HF: %04x HA: %02x LF: %02x LA: %04x", hf_l, ha_l, lf_l, la_l, hf_r, ha_r, lf_r, la_r))
      else subtree:add(f_rumble, buf(11,8), buf(11,8):uint64(), string.format("Vibration, 0x%08x 0x%08x", buf(11,4):uint(), buf(15,4):uint())) end
      
      local switch_subcmd = buf(19,1):uint()

      --Check for MCU command. These are normally used to get states and reports by demand.
      if buf(9,1):uint() == 0x11 then
        if switch_subcmd == 0x01 then  
          pkt.cols.info = "Get MCU State report?"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "MCU subcmd: 0x01 [Get MCU State? or Pairing OUT?]")
        elseif switch_subcmd == 0x03 then
          pkt.cols.info = "Get MCU IR\\NFC Input Report"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x03 [Get Input Report], Arg: 0x%02x", buf(20,1):uint()))
        else subtree:add(f_subcommand,buf(19,1))
        end  
      end

      --Check for MCU Firmware Update command.
      if buf(9,1):uint() == 0x03 then
          pkt.cols.info = "Send MCU FW Update packet" 
      end

      --Check if Command is 0x01
      if buf(9,1):uint() == 0x01 then
        --subcommand dissect
        if switch_subcmd == 0x10 then
          pkt.cols.info = "SPI Read"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x10 [SPI Read], Addr: 0x%02x%02x%02x, Size: 0x%02x", buf(22,1):uint(), buf(21,1):uint(), buf(20,1):uint(), buf(24,1):uint()))
        elseif switch_subcmd == 0x01 then
          pkt.cols.info = "Pairing OUT"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x01 [Pairing OUT], Arg: 0x%02x", buf(20,1):uint()))
        elseif switch_subcmd == 0x02 then
          pkt.cols.info = "Get Device Info"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x02 [Get Device Info]")
        elseif switch_subcmd == 0x11 then
          pkt.cols.info = "SPI Write"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x11 [SPI Write]")
        elseif switch_subcmd == 0x12 then
          pkt.cols.info = "SPI Sector Erase"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x12 [SPI Sector Erase]")
        elseif switch_subcmd == 0x08 then
          pkt.cols.info = "Set Shipment"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x08 [Set Shipment], Arg: 0x%02x", buf(20,1):uint()))
        elseif switch_subcmd == 0x03 then
          pkt.cols.info = "Set Input Report Format"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x03 [Set Input Report Format], Arg: 0x%02x", buf(20,1):uint()))
        elseif switch_subcmd == 0x04 then
          pkt.cols.info = "Get L\\R Elapsed Time"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x04 [Get L\\R Elapsed Time]")
        elseif switch_subcmd == 0x05 then
          pkt.cols.info = "Get Page info"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x05 [Get Page Info]")
        elseif switch_subcmd == 0x11 then
          pkt.cols.info = "SPI Write"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x11 [SPI Write], Addr: 0x%02x%02x%02x, Size: 0x%02x", buf(22,1):uint(), buf(21,1):uint(), buf(20,1):uint(), buf(24,1):uint()))
        elseif switch_subcmd == 0x12 then
          pkt.cols.info = "SPI Sector Erase"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x12 [SPI Sector Erase], Arg: 0x%08x %08x", buf(22,4):uint(), buf(26,4):uint()))
        elseif switch_subcmd == 0x00 then
          pkt.cols.info = "Get only Controller State"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x00 [Get only Controller State], Arg: 0x%02x%02x", buf(21,1):uint(), buf(20,1):uint()))
        elseif switch_subcmd == 0x48 then
          pkt.cols.info = "Vibration Enable"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x48 [Vibration Enable], Arg: 0x%02x", buf(20,1):uint()))
        elseif switch_subcmd == 0x40 then
          pkt.cols.info = "6-Axis Enable"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x40 [6-Axis Enable], Arg: 0x%02x", buf(20,1):uint()))
        elseif switch_subcmd == 0x30 then
          pkt.cols.info = "Set Player Lights"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x30 [Set Player Lights], Arg: 0x%02x", buf(20,1):uint()))
        elseif switch_subcmd == 0x06 then
          pkt.cols.info = "Reset Connection"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x06 [!Reset Connection!], Arg: 0x%02x", buf(20,1):uint()))
        elseif switch_subcmd == 0x21 then
          pkt.cols.info = "MCU Config Write"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x21 [MCU Config Write], Arg: %08x %08x %08x %08x %08x %08x %08x %08x %08x %02x", buf(20,4):uint(), buf(24,4):uint(), buf(28,4):uint(), buf(32,4):uint(), buf(36,4):uint(), buf(40,4):uint(), buf(44,4):uint(), buf(48,4):uint(), buf(52,4):uint(), buf(57,1):uint()))
        elseif switch_subcmd == 0x41 then
          pkt.cols.info = "Sensor Write"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x05 [Sensor Write]")
        elseif switch_subcmd == 0x42 then
          pkt.cols.info = "Sensor Config Write"
          subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x42 [Sensor Config Write], Arg: %08x %08x %08x %08x %08x %08x %08x %08x %08x %02x", buf(20,4):uint(), buf(24,4):uint(), buf(28,4):uint(), buf(32,4):uint(), buf(36,4):uint(), buf(40,4):uint(), buf(44,4):uint(), buf(48,4):uint(), buf(52,4):uint(), buf(57,1):uint()))
        elseif switch_subcmd == 0x22 then
          if buf(20,1):uint() == 0x00 then
            pkt.cols.info = "MCU Suspend"
            subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x22 00 [MCU Suspend]")
          elseif buf(20,1):uint() == 0x01 then
            pkt.cols.info = "MCU Resume"
            subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x22 01 [MCU Resume]")
          elseif buf(20,1):uint() == 0x02 then
            pkt.cols.info = "MCU Resume for Update"
            subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), "Subcmd: 0x22 02 [MCU Resume for Update]")
          else 
            pkt.cols.info = "MCU Resume set"
            subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcmd: 0x22 [MCU Resume set], Arg: 0x%02x", buf(20,1):uint()))
          end
        else subtree:add(f_subcommand,buf(19,1)) end

        subtree:add(buf(20,38), "Subcommand data")
      end

      pkt.dst = Address.ip(string.format("%02x.0.0.0", buf(0,1):uint()))
  end

end

function p_jc_btusb.init()
end

-- register chained dissector for usb packet
usb_table = DissectorTable.get("usb.bulk")
-- start after unknown interface class
usb_table:set(0xffff, p_jc_btusb)
