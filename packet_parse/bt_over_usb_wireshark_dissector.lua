-- CTCaer's Joy-Con BT over USB protocol dissect script for Wireshark
-- Version 0.8
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
local f_rumble = ProtoField.uint64("jc_btusb.rmbl", "Vibration pattern", base.HEX)

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

      local switch_subcmdack = buf(22,2):uint()

      if switch_subcmdack == 0x8202 then
        pkt.cols.info = "Device info"
        if buf(26,1):uint() == 0x01 then
          subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x02 [Device info], Left Joy-Con, FW Version: %X.%02x, MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", buf(24,1):uint(), buf(25,1):uint(), buf(28,1):uint(), buf(29,1):uint(), buf(30,1):uint(), buf(31,1):uint(), buf(32,1):uint(), buf(33,1):uint()))
        elseif buf(26,1):uint() == 0x02 then
          subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x02 [Device info], Right Joy-Con, FW Version: %X.%02x, MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x", buf(24,1):uint(), buf(25,1):uint(), buf(28,1):uint(), buf(29,1):uint(), buf(30,1):uint(), buf(31,1):uint(), buf(32,1):uint(), buf(33,1):uint()))
        end
      elseif switch_subcmdack == 0x9010 then
        pkt.cols.info = "SPI Read reply"
        subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x10 [SPI Read reply]"))
      elseif switch_subcmdack == 0x8008 then
        pkt.cols.info = "Shipment set OK"
        subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x08 [Shipment set OK]"))
      elseif switch_subcmdack == 0x8003 then
        pkt.cols.info = "Data Format set OK"
        subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x03 [Data Format set OK]"))
      elseif switch_subcmdack == 0x8040 then
        pkt.cols.info = "6-Axis Enable set OK"
        subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x40 [6-Axis Enable set OK]"))
      elseif switch_subcmdack == 0x8048 then
        pkt.cols.info = "Vibration Enable set OK"
        subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x48 [Vibration Enable set OK]"))
      elseif switch_subcmdack == 0x8030 then
        pkt.cols.info = "Player Lights set OK"
        subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x30 [Player Lights set OK]"))
      elseif switch_subcmdack == 0xA021 then
        pkt.cols.info = "MCU Config Write reply"
        subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x21 [MCU Config Write reply], Arg: %08x %08x %08x %08x %08x %08x %08x %08x %08x %02x", buf(24,4):uint(), buf(28,4):uint(), buf(32,4):uint(), buf(36,4):uint(), buf(40,4):uint(), buf(44,4):uint(), buf(48,4):uint(), buf(52,4):uint(), buf(52,4):uint(), buf(57,1):uint()))
      elseif switch_subcmdack == 0x8304 then
        pkt.cols.info = "L\\R Elapsed time"
        local time1 = 10 * bit.bor(bit.lshift(buf(25,1):uint(), 8), buf(24,1):uint())
        local time2 = 10 * bit.bor(bit.lshift(buf(27,1):uint(), 8), buf(26,1):uint())
        local time3 = 10 * bit.bor(bit.lshift(buf(29,1):uint(), 8), buf(28,1):uint())
        local time4 = 10 * bit.bor(bit.lshift(buf(31,1):uint(), 8), buf(30,1):uint())
        local time5 = 10 * bit.bor(bit.lshift(buf(33,1):uint(), 8), buf(32,1):uint())
        local time6 = 10 * bit.bor(bit.lshift(buf(35,1):uint(), 8), buf(34,1):uint())
        local time7 = 10 * bit.bor(bit.lshift(buf(37,1):uint(), 8), buf(36,1):uint())
        subtree:add(f_subcommandjoy, buf(23,1), buf(23,1):uint(), string.format("Subcmd reply: 0x04 [L\\R Elapsed time], L: %dms, R: %dms, ZL: %dms, ZR: %dms, SL: %dms, SR: %dms, HOME: %dms", time1, time2, time3, time4, time5, time6, time7))
      else subtree:add(f_subcommandjoy,buf(23,1))
      end

      pkt.src = Address.ip(string.format("%02x.0.0.0", buf(0,1):uint()))
      
  --Host packet
  elseif buf(1,1):uint() == 0x00
    then
      subtree:add(f_dst, buf(0,1))
      subtree:add(f_command, buf(9,1))
      subtree:add(f_timming, buf(10,1))
      --Vibration command and pattern
      if buf(9,1):uint() == 0x10 then 
        pkt.cols.info = "Vibration set"
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
      else subtree:add(f_rumble, buf(11,8)) end
      
      local switch_subcmd = buf(19,1):uint()

      --subcommand dissect
      if switch_subcmd == 0x10 then
        pkt.cols.info = "SPI Read"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x10 [SPI Read], Addr: 0x%02x%02x%02x, Size: 0x%02x", buf(22,1):uint(), buf(21,1):uint(), buf(20,1):uint(), buf(24,1):uint()))
      elseif switch_subcmd == 0x01 then
        pkt.cols.info = "Pairing OUT"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x01 [Pairing OUT], Arg: 0x%02x", buf(20,1):uint()))
      elseif switch_subcmd == 0x02 then
        pkt.cols.info = "Get Device Info"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x02 [Get Device Info]"))
      elseif switch_subcmd == 0x08 then
        pkt.cols.info = "Set Shipment"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x08 [Set Shipment], Arg: 0x%02x", buf(20,1):uint()))
      elseif switch_subcmd == 0x03 then
        pkt.cols.info = "Set Data Report Format"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x03 [Set Data Report Format], Arg: 0x%02x", buf(20,1):uint()))
      elseif switch_subcmd == 0x04 then
        pkt.cols.info = "Get L\\R Elapsed Time"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x04 [Get L\\R Elapsed Time]"))
      elseif switch_subcmd == 0x00 then
        pkt.cols.info = "Pairing IN"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x00 [Pairing IN], Arg: 0x%02x%02x", buf(21,1):uint(), buf(20,1):uint()))
      elseif switch_subcmd == 0x48 then
        pkt.cols.info = "Vibration Enable"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x48 [Vibration Enable], Arg: 0x%02x", buf(20,1):uint()))
      elseif switch_subcmd == 0x40 then
        pkt.cols.info = "6-Axis Enable"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x40 [6-Axis Enable], Arg: 0x%02x", buf(20,1):uint()))
      elseif switch_subcmd == 0x30 then
        pkt.cols.info = "Set Player Lights"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x30 [Set Player Lights], Arg: 0x%02x", buf(20,1):uint()))
      elseif switch_subcmd == 0x06 then
        pkt.cols.info = "Reset Connection"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x06 [!Reset Connection!], Arg: 0x%02x", buf(20,1):uint()))
      elseif switch_subcmd == 0x21 then
        pkt.cols.info = "MCU Config Write"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x21 [MCU Config Write], Arg: %08x %08x %08x %08x %08x %08x %08x %08x %08x %02x", buf(20,4):uint(), buf(24,4):uint(), buf(28,4):uint(), buf(32,4):uint(), buf(36,4):uint(), buf(40,4):uint(), buf(44,4):uint(), buf(48,4):uint(), buf(52,4):uint(), buf(57,1):uint()))
      elseif switch_subcmd == 0x22 then
        pkt.cols.info = "MCU Resume"
        subtree:add(f_subcommand, buf(19,1), buf(19,1):uint(), string.format("Subcommand: 0x22 [MCU Resume], Arg: 0x%02x", buf(20,1):uint()))
      else subtree:add(f_subcommand,buf(19,1))
      end

      subtree:add(buf(20,38), "Subcommand data")

      pkt.dst = Address.ip(string.format("%02x.0.0.0", buf(0,1):uint()))
  end

end

function p_jc_btusb.init()
end

-- register chained dissector for usb packet
usb_table = DissectorTable.get("usb.bulk")
-- start after unknown interface class
usb_table:set(0xffff, p_jc_btusb)
