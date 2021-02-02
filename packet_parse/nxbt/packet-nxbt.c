
#include <config.h>
#include <epan/packet.h>

#define NAMES_END { 0x00, NULL }

// "this" as handles
static int proto_nxbt = -1;
static gint ett_nxbt = -1;

// handles for all fields in the protocoll

/**
 * How to extend this:
 * Field xyz is not shown in Wireshark:
 * - add a handle
 * - add a register
 * - add the register to the list in proto_register
 * - call proto_tree_add_item at some point
 * - don't fuck up the cursor
 */


// general header
static int hf_nxbt_direction = -1;
static const value_string direction_type_names[] = {
  { 0xa2, "Output" },
  { 0xa1, "Input"},
  NAMES_END
};
#define hf_nxbt_direction_register \
{ \
  &hf_nxbt_direction, \
  { \
    "first byte", "nxbt.first", \
    FT_UINT8, BASE_HEX, \
    VALS(direction_type_names), 0x0, \
    NULL, HFILL \
  } \
}

static int hf_nxbt_type = -1;
static const value_string type_names[] = {
  // Output types
    { 0x01, "Rumble and subc" },
    { 0x03, "NFC/IR Update" },
    { 0x10, "Rumble" },
    { 0x11, "Rumble and NFC/IR request" },
    { 0x12, "Unknown, similar to 0x28" },

  //Input types
    { 0x21, "subc reply"},
    { 0x23, "MCU firmware update"},
    { 0x30, "Standard input record"},
    { 0x31, "input and MCU data"},
    { 0x32, "Unknown, looks standard"},
    { 0x33, "Unknown, looks standard"},
    { 0x3F, "Pure BT HID"},
    NAMES_END
};
#define hf_nxbt_type_register { \
  &hf_nxbt_type, \
  { \
    "Report type", "nxbt.type", \
    FT_UINT8, BASE_HEX, \
    VALS(type_names), 0x0, \
    NULL, HFILL \
  } \
}

static int hf_nxbt_timer = -1;
#define hf_nxbt_timer_register { \
  &hf_nxbt_timer, \
  { \
    "timer", "nxbt.timer", \
    FT_UINT8, BASE_HEX, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}


//rumble stuff
static int nxbt_rumble = -1;
static int nxbt_rumble_tree = -1;
#define nxbt_rumble_register { \
  &nxbt_rumble, \
  { \
    "Rumble data", "nxbt.rumble", \
    FT_NONE, ENC_NA, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_rumble_left = -1;
#define nxbt_rumble_left_register { \
  &nxbt_rumble_left, \
  { \
    "Left rumble", "nxbt.rumble.left", \
    FT_UINT32, BASE_HEX, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_rumble_right = -1;
#define nxbt_rumble_right_register { \
  &nxbt_rumble_right, \
  { \
    "Right rumble", "nxbt.rumble.right", \
    FT_UINT32, BASE_HEX, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int dissect_nxbt_rumble(tvbuff_t* tvb, packet_info* pinfo _U_, proto_tree* nxbt_tree _U_, void* data _U_, uint cursor) {
  proto_item* rumble_item = proto_tree_add_none_format(nxbt_tree, nxbt_rumble, tvb, cursor, 8, "Rumble data");
  proto_tree* rumble_tree = proto_item_add_subtree(rumble_item, nxbt_rumble_tree);
  proto_tree_add_item(rumble_tree, nxbt_rumble_left, tvb, cursor, 4, ENC_BIG_ENDIAN);
  cursor += 4;
  proto_tree_add_item(rumble_tree, nxbt_rumble_right, tvb, cursor, 4, ENC_BIG_ENDIAN);
  col_set_str(pinfo->cinfo, COL_INFO, "rumble data");
  return cursor + 4;
}

//spi stuff
static int nxbt_spi_address = -1;
#define nxbt_spi_address_register { \
  &nxbt_spi_address, \
  { \
    "Address", "nxbt.sub.spi.address", \
    FT_UINT32, BASE_HEX, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_spi_length = -1;
#define nxbt_spi_length_register { \
  &nxbt_spi_length, \
  { \
    "Amount", "nxbt.sub.spi.length", \
    FT_UINT8, BASE_DEC, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_spi_data = -1;
#define nxbt_spi_data_register { \
  &nxbt_spi_data, \
  { \
    "data", "nxbt.sub.spi.data", \
    FT_BYTES, SEP_SPACE, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int dissect_nxbt_spi(tvbuff_t* tvb, packet_info* pinfo _U_, proto_tree* tree _U_, void* data _U_, uint cursor, char has_data) {
  proto_tree_add_item(tree, nxbt_spi_address, tvb, cursor, 4, ENC_LITTLE_ENDIAN);
  cursor += 4;
  proto_tree_add_item(tree, nxbt_spi_length, tvb, cursor++, 1, ENC_LITTLE_ENDIAN);
  if (has_data) {
    guint8 length = tvb_get_guint8(tvb, cursor-1);
    proto_tree_add_item(tree, nxbt_spi_data, tvb, cursor, length, ENC_LITTLE_ENDIAN);
    cursor += length;
  }
  return cursor;
}


// subcommands
static int nxbt_subc = -1;
static int nxbt_subc_tree = -1;
#define nxbt_subc_register { \
  &nxbt_subc, \
  { \
    "Subcommand", "nxbt.sub", \
    FT_NONE, ENC_NA, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_subc_c = -1;
static const value_string nxbt_subc_c_names[] = {
  { 0x01, "Manual Pairing"},
  { 0x02, "Get device info"},
  { 0x03, "Set input Mode"},
  { 0x04, "Trigger buttons elapsed time"},
  { 0x05, "Get page list state"},
  { 0x06, "Set HCI state"},
  { 0x07, "Reset pairing info"},
  { 0x08, "Set shipment low power state"},
  { 0x10, "SPI flash read"},
  { 0x11, "SPI flash Write"},
  { 0x12, "SPI sector erase"},
  { 0x20, "Reset NFC/IR MCU"},
  { 0x21, "Set NFC/IR MCU configuration"},
  { 0x22, "Set NFC/IR MCU state"},
  { 0x24, "Set unknown data (fw 3.86 and up)"},
  { 0x25, "Reset 0x24 unknown data (fw 3.86 and up)"},
  { 0x28, "Set unknown NFC/IR MCU data A"},
  { 0x29, "Get unknown NFC/IR MCU data A"},
  { 0x2A, "Set GPIO Pin Output value (2 @Port 2)"},
  { 0x2B, "Get x29 NFC/IR MCU data"},
  { 0x30, "Set player lights"},
  { 0x31, "Get player lights"},
  { 0x38, "Set HOME Light"},
  { 0x40, "Enable IMU (6-Axis sensor)"},
  { 0x41, "Set IMU sensitivity"},
  { 0x42, "Write to IMU registers"},
  { 0x43, "Read IMU registers"},
  { 0x48, "Enable vibration"},
  { 0x50, "Get regulated voltage"},
  { 0x51, "Set GPIO Pin Output value (7 & 15 @Port 1)"},
  { 0x52, "Get GPIO Pin Input/Output value"},
  NAMES_END
};
#define nxbt_subc_c_register { \
  &nxbt_subc_c, \
  { \
    "Subcommand", "nxbt.sub.c", \
    FT_UINT8, BASE_HEX, \
    VALS(nxbt_subc_c_names), 0x0, \
    NULL, HFILL \
  } \
}

// set mcu config
static int nxbt_subc_mcu_config = -1;
#define nxbt_subc_mcu_config_register { \
  &nxbt_subc_mcu_config, \
  { \
    "Powerstate configuration", "nxbt.sub.mcu.config", \
    FT_UINT8, BASE_HEX, \
    VALS(nxbt_mcu_power_state_names), 0x0, \
    NULL, HFILL \
  } \
}

//set mcu state
static int nxbt_subc_MCU_state = -1;
static const value_string mcu_state_names[] = {
  { 0x00, "Suspended"},
  { 0x01, "Active"},
  { 0x02, "Active for Update"},
  NAMES_END
};
#define nxbt_subc_MCU_state_register { \
  &nxbt_subc_MCU_state, \
  { \
    "State to go into", "nxbt.sub.mcu.state", \
    FT_UINT8, BASE_DEC, \
    VALS(mcu_state_names), 0x0, \
    NULL, HFILL \
  } \
}

//set player lights
static int nxbt_subc_player_lights = -1;
#define nxbt_subc_player_lights_register { \
  &nxbt_subc_player_lights, \
  { \
    "Player Lights", "nxbt.sub.player", \
    FT_UINT8, BASE_OCT, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

//set input report type
static int nxbt_subc_type = -1;
#define nxbt_subc_type_register { \
  &nxbt_subc_type, \
  { \
    "Report type to switch to", "nxbt.sub.type", \
    FT_UINT8, BASE_HEX, \
    VALS(type_names), 0x0, \
    NULL, HFILL \
  } \
}

static int dissect_nxbt_subc(tvbuff_t* tvb, packet_info* pinfo _U_, proto_tree* nxbt_tree _U_, void* data _U_, uint cursor) {
  uint start = cursor;
  proto_item* subc_item = proto_tree_add_none_format(nxbt_tree, nxbt_subc, tvb, cursor, -1, "Subcommand");
  proto_tree* subc_tree = proto_item_add_subtree(subc_item, nxbt_subc_tree);
  guint8 subc_c = tvb_get_guint8(tvb, cursor);
  proto_tree_add_item(subc_tree, nxbt_subc_c, tvb, cursor++, 1, ENC_BIG_ENDIAN);
  switch (subc_c) {
    case 0x03: // set input mode
      proto_tree_add_item(subc_tree, nxbt_subc_type, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      break;
    case 0x10: //SPI read
    case 0x11: //SPI write
      cursor = dissect_nxbt_spi(tvb, pinfo, subc_tree, data, cursor, subc_c == 0x11);
      break;
    case 0x20: //reset MCU
      break;
    case 0x21: //set MCU config
      cursor += 2;
      proto_tree_add_item(subc_tree, nxbt_subc_mcu_config, tvb, cursor++, 1, ENC_LITTLE_ENDIAN);
      cursor += 34;
      break;
    case 0x22: // Set MCU state
      proto_tree_add_item(subc_tree, nxbt_subc_MCU_state, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      break;
    case 0x30: // set player lights
      proto_tree_add_item(subc_tree, nxbt_subc_player_lights, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      break;
  }
  proto_item_set_len(subc_item, cursor - start);
  col_set_str(pinfo->cinfo, COL_INFO, val_to_str(subc_c, nxbt_subc_c_names, "unknown Subcommand %04x"));
  return cursor;
}



// subcommand replies
static int nxbt_rep = -1;
static int nxbt_rep_tree = -1;
#define nxbt_rep_register { \
  &nxbt_rep, \
  { \
    "Subcommand Reply", "nxbt.rep", \
    FT_NONE, ENC_NA, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

#define NXBT_REP_ACK_FLAG (1 << 7)
static int nxbt_rep_ack = -1;
static const true_false_string nxbt_rep_ack_names = {
    "ACK",
    "NACK"
};
#define nxbt_rep_ack_register { \
  &nxbt_rep_ack, \
  { \
    "ack flag", "nxbt.rep.ack", \
    FT_BOOLEAN, 8, \
    TFS(&nxbt_rep_ack_names), NXBT_REP_ACK_FLAG, \
    NULL, HFILL \
  } \
}

#define NXBT_REP_DTYPE_MASK ((guint8) ~NXBT_REP_ACK_FLAG)
static int nxbt_rep_dtype = -1;
#define nxbt_rep_dtype_register { \
  &nxbt_rep_dtype, \
  { \
    "reply dtype", "nxbt.rep.dtype", \
    FT_UINT8, BASE_HEX, \
    NULL, NXBT_REP_DTYPE_MASK, \
    NULL, HFILL \
  } \
}

static int nxbt_rep_subc = -1;
#define nxbt_rep_subc_register { \
  &nxbt_rep_subc, \
  { \
    "response to", "nxbt.rep.sub", \
    FT_UINT8, BASE_HEX, \
    VALS(nxbt_subc_c_names), 0x0, \
    NULL, HFILL \
  } \
}

static int dissect_nxbt_subc_reply(tvbuff_t* tvb, packet_info* pinfo _U_, proto_tree* nxbt_tree _U_, void* data _U_, uint cursor) {
  guint8 ack = tvb_get_guint8(tvb, cursor) & NXBT_REP_ACK_FLAG;
  guint8 subc_c = tvb_get_guint8(tvb, cursor + 1);
  const gchar* subc_name = val_to_str(subc_c, nxbt_subc_c_names, "unknown subcommand %04x");
  proto_item* rep_item = proto_tree_add_none_format(nxbt_tree, nxbt_rep, tvb, cursor, 8,
     "Subcommand %s %s", subc_name, ack ? "ACK" : "NACK");
  proto_tree* rep_tree = proto_item_add_subtree(rep_item, nxbt_rep_tree);
  proto_tree_add_item(rep_tree, nxbt_rep_ack, tvb, cursor, 1, ENC_BIG_ENDIAN);
  proto_tree_add_item(rep_tree, nxbt_rep_dtype, tvb, cursor++, 1, ENC_BIG_ENDIAN);
  proto_tree_add_item(rep_tree, nxbt_rep_subc, tvb, cursor++, 1, ENC_BIG_ENDIAN);
  switch (subc_c) {
    case 0x10:
    case 0x11:
      cursor = dissect_nxbt_spi(tvb, pinfo, rep_tree, data, cursor, subc_c != 0x11);
      break;
  }
  col_add_fstr(pinfo->cinfo, COL_INFO, ack ? "ACK %s" : "NACK %s", subc_name);
  cursor += 34;
  return cursor;
}



// General MCU stuff
static int nxbt_mcu = -1;
static int nxbt_mcu_tree = -1;
#define nxbt_mcu_register { \
  &nxbt_mcu, \
  { \
    "MCU Data", "nxbt.mcu", \
    FT_NONE, ENC_NA, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

// command and first are identical just on different sides
static int nxbt_mcu_c = -1;
static const value_string nxbt_mcu_c_names[] = {
  {0x01, "Status request"},
  {0x02, "NFC subsubcommand"},
  NAMES_END
};
#define nxbt_mcu_c_register { \
  &nxbt_mcu_c, \
  { \
    "command", "nxbt.mcu.c", \
    FT_UINT8, BASE_HEX, \
    VALS(nxbt_mcu_c_names), 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_mcu_first = -1;
static const value_string nxbt_mcu_first_names[] = {
  {0x01, "MCU status"},
  {0x2a, "NFC status"},
  {0x3a, "NFC data buffered"},
  {0xff, "No response/MCU disables"},
  NAMES_END
};
#define nxbt_mcu_first_register { \
  &nxbt_mcu_first, \
  { \
    "type of MCU message", "nxbt.mcu.first", \
    FT_UINT8, BASE_HEX, \
    VALS(nxbt_mcu_first_names), 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_mcu_seqno = -1;
#define nxbt_mcu_seqno_register { \
  &nxbt_mcu_seqno, \
  { \
    "Sequence no", "nxbt.mcu.seq", \
    FT_UINT8, BASE_DEC, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_mcu_ackseqno = -1;
#define nxbt_mcu_ackseqno_register { \
  &nxbt_mcu_ackseqno, \
  { \
    "[ack-seqno]", "nxbt.mcu.ackseqno", \
    FT_UINT8, BASE_DEC, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

#define NXBT_MCUC_EOT_FLAG 0x08
static int nxbt_mcu_eot = -1;
static const true_false_string nxbt_mcu_eot_names = {
    "EOT",
    "MORE"
};
#define nxbt_mcu_eot_register { \
  &nxbt_mcu_eot, \
  { \
    "eot flag", "nxbt.mcu.eot", \
    FT_BOOLEAN, 8, \
    TFS(&nxbt_mcu_eot_names), NXBT_MCUC_EOT_FLAG, \
    NULL, HFILL \
  } \
}

static int nxbt_mcu_payload_len = -1;
#define nxbt_mcu_payload_len_register { \
  &nxbt_mcu_payload_len, \
  { \
    "Payload_length", "nxbt.mcu.data.len", \
    FT_UINT8, BASE_DEC, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_mcu_crc = -1;
#define nxbt_mcu_crc_register { \
  &nxbt_mcu_crc, \
  { \
    "MCU crc", "nxbt.mcu.crc", \
    FT_UINT8, BASE_HEX, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}


// MCU out
static int nxbt_mcu_nfcc = -1;
static const value_string nxbt_mcu_nfcc_names[] = {
  {0x01, "Start Polling"},
  {0x02, "Stop Polling"},
  {0x04, "Get next Data / Status"},
  {0x06, "Read and buffer NTag"},
  {0x08, "[Write to NTag]"},
  {0x0f, "Read mifare data"},
  NAMES_END
};
#define nxbt_mcu_nfcc_register { \
  &nxbt_mcu_nfcc, \
  { \
    "Subcommand for NFC", "nxbt.mcu.nfc.c", \
    FT_UINT8, BASE_HEX, \
    VALS(nxbt_mcu_nfcc_names), 0x0, \
    NULL, HFILL \
  } \
}

static int counter = 1;

static int dissect_nxbt_mcu_out(tvbuff_t* tvb, packet_info* pinfo _U_, proto_tree* nxbt_tree _U_, void* data _U_, uint cursor) {
  uint start = cursor;
  proto_item* mcu_item = proto_tree_add_none_format(nxbt_tree, nxbt_mcu, tvb, cursor, -1, "Mcu Command");
  proto_tree* mcu_tree = proto_item_add_subtree(mcu_item, nxbt_mcu_tree);
  guint8 mcu_c = tvb_get_guint8(tvb, cursor);
  proto_tree_add_item(mcu_tree, nxbt_mcu_c, tvb, cursor++, 1, ENC_BIG_ENDIAN);
  if (mcu_c == 0x02) { // NFC
    guint8 nfcc = tvb_get_guint8(tvb, cursor);
    proto_tree_add_item(mcu_tree, nxbt_mcu_nfcc, tvb, cursor++, 1, ENC_BIG_ENDIAN);
    proto_tree_add_item(mcu_tree, nxbt_mcu_seqno, tvb, cursor++, 1, ENC_BIG_ENDIAN);
    proto_tree_add_item(mcu_tree, nxbt_mcu_ackseqno, tvb, cursor++, 1, ENC_BIG_ENDIAN);
    proto_tree_add_item(mcu_tree, nxbt_mcu_eot, tvb, cursor++, 1, ENC_BIG_ENDIAN);
    //guint8 payload_len = tvb_get_guint8(tvb, cursor);
    proto_tree_add_item(mcu_tree, nxbt_mcu_payload_len, tvb, cursor++, 1, ENC_BIG_ENDIAN);
    //cursor += payload_len;
    cursor += 31;
    proto_tree_add_item(mcu_tree, nxbt_mcu_crc, tvb, cursor++, 1, ENC_BIG_ENDIAN);
    col_add_fstr(pinfo->cinfo, COL_INFO, "MCU-NFC command %s %d", val_to_str(nfcc, nxbt_mcu_nfcc_names, "unknown 0x%02x"), ++counter);
  } else {
    col_add_fstr(pinfo->cinfo, COL_INFO, "MCU command %s", val_to_str(mcu_c, nxbt_mcu_c_names, "unknown 0x%02x"));
  }
  proto_item_set_len(mcu_item, cursor - start);
  return cursor;
}


// MCU in
static int nxbt_mcu_power_state = -1;
static const value_string nxbt_mcu_power_state_names[] = {
  { 0x00, "Suspended" },
  { 0x01, "Ready" },
  { 0x02, "Ready for Update" },
  { 0x04, "Configured NFC" },
  { 0x05, "Configured IR" },
  { 0x06, "Configured Update" },
  NAMES_END
};
#define nxbt_mcu_power_state_register { \
  &nxbt_mcu_power_state, \
  { \
    "MCU's powerstate", "nxbt.mcu.state", \
    FT_UINT8, BASE_HEX, \
    VALS(nxbt_mcu_power_state_names), 0x0, \
    NULL, HFILL \
  } \
}

//static int nxbt_mcu_error = -1;

//static int nxbt_mcu_nfc_type = -1;

static int nxbt_mcu_nfc_state = -1;
static const value_string nxbt_mcu_nfc_state_names[] = {
  { 0x00, "None" },
  { 0x01, "Polled" },
  { 0x02, "Buffered data / Pending read" },
  { 0x03, "[Writing]"},
  { 0x09, "Polled, found tag again" },
  NAMES_END
};
#define nxbt_mcu_nfc_state_register { \
  &nxbt_mcu_nfc_state, \
  { \
    "NFC subsystem state", "nxbt.mcu.nfc.state", \
    FT_UINT8, BASE_HEX, \
    VALS(nxbt_mcu_nfc_state_names), 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_mcu_nfc_uuid = -1;
#define nxbt_mcu_nfc_uuid_register { \
  &nxbt_mcu_nfc_uuid, \
  { \
    "UUID of detected tag", "nxbt.mcu.nfc.uuid", \
    FT_BYTES, SEP_COLON, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int nxbt_mcu_nfc_data = -1;
#define nxbt_mcu_nfc_data_register { \
  &nxbt_mcu_nfc_data, \
  { \
    "Nfc tag data transmitted", "nxbt.mcu.nfc.data", \
    FT_NONE, ENC_NA, \
    NULL, 0x0, \
    NULL, HFILL \
  } \
}

static int dissect_nxbt_mcu_in(tvbuff_t* tvb, packet_info* pinfo _U_, proto_tree* nxbt_tree _U_, void* data _U_, uint cursor) {
  proto_item* mcu_item = proto_tree_add_none_format(nxbt_tree, nxbt_mcu, tvb, cursor, 313,
     "MCU response");
  proto_tree* mcu_tree = proto_item_add_subtree(mcu_item, nxbt_mcu_tree);
  guint8 first = tvb_get_guint8(tvb, cursor);
  proto_tree_add_item(mcu_tree, nxbt_mcu_first, tvb, cursor++, 1, ENC_BIG_ENDIAN);
  guint8 seqno = 0;
  //guint8 mcu_seqno = 0;
  guint8 payload_len = 0;
  switch (first) {
    case 0x01:
      cursor += 6;
      proto_tree_add_item(mcu_tree, nxbt_mcu_power_state, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      col_set_str(pinfo->cinfo, COL_INFO, "MCU status");
      cursor += 304;
      break;
    case 0x2a:
      cursor += 2;
      proto_tree_add_item(mcu_tree, nxbt_mcu_seqno, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      proto_tree_add_item(mcu_tree, nxbt_mcu_ackseqno, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      cursor += 2;
      proto_tree_add_item(mcu_tree, nxbt_mcu_nfc_state, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      cursor += 7;
      payload_len = tvb_get_guint8(tvb, cursor);
      proto_tree_add_item(mcu_tree, nxbt_mcu_payload_len, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      proto_tree_add_item(mcu_tree, nxbt_mcu_nfc_uuid, tvb, cursor, 7, ENC_BIG_ENDIAN);
      cursor += 7;
      cursor += 289;
      col_set_str(pinfo->cinfo, COL_INFO, payload_len == 0 ? "NFC status" : "NFC status with payload");
      break;
    case 0x3a:
      cursor += 2;
      seqno = tvb_get_guint8(tvb, cursor);
      proto_tree_add_item(mcu_tree, nxbt_mcu_seqno, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      proto_tree_add_item(mcu_tree, nxbt_mcu_ackseqno, tvb, cursor++, 1, ENC_BIG_ENDIAN);
      if (seqno == 1) {
        cursor += 2;
        proto_tree_add_item(mcu_tree, nxbt_mcu_nfc_state, tvb, cursor++, 1, ENC_BIG_ENDIAN);
        cursor += 6;
        proto_tree_add_item(mcu_tree, nxbt_mcu_payload_len, tvb, cursor++, 1, ENC_BIG_ENDIAN);
        proto_tree_add_item(mcu_tree, nxbt_mcu_nfc_uuid, tvb, cursor, 7, ENC_BIG_ENDIAN);
        cursor += 7;
        cursor += 45;
        proto_tree_add_none_format(mcu_tree, nxbt_mcu_nfc_data, tvb, cursor, 245, "Raw NFC tag data");
        cursor += 245;
        col_set_str(pinfo->cinfo, COL_INFO, "NFC read buffered data #1");
      } else if (seqno == 2) {
        cursor += 5;
        proto_tree_add_none_format(mcu_tree, nxbt_mcu_nfc_data, tvb, cursor, 295, "Raw NFC tag data");
        cursor += 295;
        cursor += 7;
        col_set_str(pinfo->cinfo, COL_INFO, "NFC read buffered data #2");
      }
      break;
    case 0xff:
      cursor += 311;
      break;
  }
  proto_tree_add_item(mcu_tree, nxbt_mcu_crc, tvb, cursor, 1, ENC_BIG_ENDIAN);
  return cursor;
}


static int dissect_nxbt(tvbuff_t* tvb, packet_info* pinfo, proto_tree* tree _U_, void* data _U_) {
  col_set_str(pinfo->cinfo, COL_PROTOCOL, "NX BT");
  col_clear(pinfo->cinfo, COL_INFO);

  // the root node in the details view for our stuff
	proto_item* ti = proto_tree_add_item(tree, proto_nxbt, tvb, 0, -1, ENC_NA);
  proto_tree *nxbt_tree = proto_item_add_subtree(ti, ett_nxbt);

  unsigned int cursor = 0;

  // header
  proto_tree_add_item(nxbt_tree, hf_nxbt_direction, tvb, cursor++, 1, ENC_BIG_ENDIAN);
  proto_tree_add_item(nxbt_tree, hf_nxbt_type, tvb, cursor++, 1, ENC_BIG_ENDIAN);
  proto_tree_add_item(nxbt_tree, hf_nxbt_timer, tvb, cursor++, 1, ENC_BIG_ENDIAN);
  guint type = tvb_get_guint8(tvb, 1);

  // Output
  if (type == 0x01 || type == 0x10 || type == 0x11) { //have rumble
    cursor = dissect_nxbt_rumble(tvb, pinfo, nxbt_tree, data, cursor);
  }
  if (type == 0x01) {
    cursor = dissect_nxbt_subc(tvb, pinfo, nxbt_tree, data, cursor);
    //proto_tree_add_item(nxbt_tree, nxbt_subc_data, tvb, cursor, 1, ENC_BIG_ENDIAN);
  } else if (type == 0x11) {
    cursor = dissect_nxbt_mcu_out(tvb, pinfo, nxbt_tree, data, cursor);
  }

  // Input
  if (type == 0x30 || type == 0x31 || type == 0x31 || type == 0x32) {
    // TODO parse input
    cursor += 1+3+3+3+1;
  } else if (type == 0x21 || type == 0x23) { // no input data
    cursor += 1+3+3+3+1;
  }
  if (type == 0x21) { // subcommand reply
    cursor = dissect_nxbt_subc_reply(tvb, pinfo, nxbt_tree, data, cursor);
  } else if (type == 0x23) {
    // NFC stuff
    cursor += 36;
  } else if (type == 0x30 || type == 0x31 || type == 0x32 || type == 0x33) {
    // TODO 6 Axis data
    cursor += 36;
  }
  if (type == 0x31) {
    cursor = dissect_nxbt_mcu_in(tvb, pinfo, nxbt_tree, data, cursor);
  }
  return tvb_captured_length(tvb);
}

void proto_register_nxbt(void) {
  static hf_register_info hf[] = {
    hf_nxbt_direction_register,
		hf_nxbt_type_register,
    hf_nxbt_timer_register,

    nxbt_rumble_register,
    nxbt_rumble_left_register,
    nxbt_rumble_right_register,

    nxbt_spi_address_register,
    nxbt_spi_length_register,
    nxbt_spi_data_register,

    nxbt_subc_register,
    nxbt_subc_c_register,
    nxbt_subc_mcu_config_register,
    nxbt_subc_MCU_state_register,
    nxbt_subc_player_lights_register,
    nxbt_subc_type_register,

    nxbt_rep_register,
    nxbt_rep_ack_register,
    nxbt_rep_dtype_register,
    nxbt_rep_subc_register,

    nxbt_mcu_register,
    nxbt_mcu_c_register,
    nxbt_mcu_first_register,
    nxbt_mcu_seqno_register,
    nxbt_mcu_ackseqno_register,
    nxbt_mcu_eot_register,
    nxbt_mcu_payload_len_register,
    nxbt_mcu_crc_register,

    nxbt_mcu_nfcc_register,

    nxbt_mcu_power_state_register,
    nxbt_mcu_nfc_state_register,
    nxbt_mcu_nfc_uuid_register,
    nxbt_mcu_nfc_data_register,

	};

  static gint* ett[] = {
    &ett_nxbt,
    &nxbt_rumble_tree,
    &nxbt_subc_tree,
    &nxbt_mcu_tree,
    &nxbt_rep_tree,
  };

	proto_nxbt = proto_register_protocol(
		"Nintendo Switch Bluetooth Controller Kommunikation",
		"NX_BT",
		"nxbt"
	);

  proto_register_field_array(proto_nxbt, hf, array_length(hf));
  proto_register_subtree_array(ett, array_length(ett));
}

void proto_reg_handoff_nxbt(void) {
	static dissector_handle_t nxbt_handle;
	nxbt_handle = create_dissector_handle(dissect_nxbt, proto_nxbt);
	//dissector_add_string("bluetooth.src", SWITCH_MAC, nxbl_handle);
	//dissector_add_string("bluetooth.dest", SWITCH_MAC, nxbl_handle);
	//dissector_add_uint("llc.bluetooth_pid", 1, nxbl_handle); // overwrite L2CAP
	dissector_add_uint("btl2cap.psm", 17, nxbt_handle);
	dissector_add_uint("btl2cap.psm", 19, nxbt_handle);
	//dissector_add_for_decode_as("btl2cap.cid", nxbt_handle);
}
