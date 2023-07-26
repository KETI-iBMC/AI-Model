#include <ipmi/KETI_stdx.hpp>
#include <redfish/resource.hpp>
#include "handler.hpp"
extern unordered_map<string, Resource *> g_record;
extern ServiceRoot *g_service_root;
extern map<int, set<unsigned int>> numset;
extern map<int, unsigned int> numset_num;

/**
 * @brief Resource initialization
*  @bug 기철 - wrong in save json 해결중 ..
 */
bool init_resource(void) {
  init_numset();
  record_load_json();
  log(info) << "[Record Load Json] User" << endl;


 
  for (int i = 0; i < get_user_cnt(); i++) {
    ipmiUser[i].printUserInfo();
  }

  log(info) << "[Record Load Json] complete" << endl;

  if (!record_is_exist(ODATA_SERVICE_ROOT_ID))
    g_service_root = new ServiceRoot();


  init_message_registry();

  record_save_json();
  log(info) << "[Record Save Json] complete" << endl;

  return true;
}

void init_system(Systems *system, string _id) {
  string odata_id = system->odata.id;
  /**
   * Computer System Configuration
   */
  system->fru_this = &fru_rec.find(0)->second.find(0)->second;
  system->id = _id;
  system->name = std::string((char *)system->fru_this->fru_board.product);
  // system->sku = "";
  system->system_type = "Physical";
  system->asset_tag = std::string((char *)system->fru_this->product.asset_tag);
  system->manufacturer = "KETI";
  system->model = "";
  system->serial_number =
      std::string((char *)system->fru_this->fru_chassis.serial);
  system->part_number =
      std::string((char *)system->fru_this->fru_chassis.part_number);
  system->description = "";
  system->uuid = string(uuid_str);
  system->hostname = ipmiApplication.g_host_name;
  // system->hosting_roles.push_back("null");
  system->indicator_led = LED_OFF;
  system->power_state = ipmiChassis.get_power_status() ? "On" : "Off";
  system->bios_version = "";

  system->status.state = STATUS_STATE_ENABLED;
  system->status.health = STATUS_HEALTH_OK;

  // Boot
  system->boot.uefi_target_boot_source_override = "None";
  system->boot.boot_source_override_target = "Legacy";
  system->boot.boot_source_override_mode = "";

  uint8_t enable;

  if (ipmiChassis.g_chassis_bootp.boot_flags.size() > 0) {
    enable = (ipmiChassis.g_chassis_bootp.boot_flags.at(0) & BF1_VALID) ? 1 : 0;
    if (enable == 1)
      system->boot.boot_source_override_enabled =
          (ipmiChassis.g_chassis_bootp.boot_flags.at(0) & BF1_VALID &
           BF1_PERSIST)
              ? "Continuous"
              : "Once";
    else
      system->boot.boot_source_override_enabled = "Disabled";

    switch (ipmiChassis.g_chassis_bootp.boot_flags.at(1) & BF1_VALID &
            BF2_BOOTDEV_MASK) {
    case BF2_BOOTDEV_DEFAULT:
      system->boot.boot_source_override_enabled = "None";
      break;
    case BF2_BOOTDEV_PXE:
      system->boot.boot_source_override_enabled = "Pxe";
      break;
    case BF2_BOOTDEV_HDD:
      system->boot.boot_source_override_enabled = "Hdd";
      break;
    case BF2_BOOTDEV_DIAG_PART:
      system->boot.boot_source_override_enabled = "Diags";
      break;
    case BF2_BOOTDEV_CDROM:
      system->boot.boot_source_override_enabled = "Cd";
      break;
    case BF2_BOOTDEV_SETUP:
      system->boot.boot_source_override_enabled = "BiosSetup";
      break;
    case BF2_BOOTDEV_REMOTE_CDROM:
      system->boot.boot_source_override_enabled = "RemoteDrive";
      break;
    case BF2_BOOTDEV_FDD:
      system->boot.boot_source_override_enabled = "Floppy";
      break;
    default:
      system->boot.boot_source_override_enabled = "None";
      break;
    }
  } else {
    system->boot.boot_source_override_enabled = "None";
  }

  if (!record_is_exist(odata_id + "/Storage")) {
    log(info) << "[...]System storage init";
    system->storage =
        new Collection(odata_id + "/Storage", ODATA_STORAGE_COLLECTION_TYPE);
    system->storage->name = "Computer System Storage Collection";

    init_storage_collection(system->storage, "1");
  }
  if (!record_is_exist(odata_id + "/Processors")) {
    log(info) << "[...]System processor init";
    system->processor = new Collection(odata_id + "/Processors",
                                       ODATA_PROCESSOR_COLLECTION_TYPE);
    system->processor->name = "Computer System Processor Collection";

    init_processor(system->processor, "CPU1");
  }
  if (!record_is_exist(odata_id + "/Memory")) {
    log(info) << "[...]System memory init";
    system->memory =
        new Collection(odata_id + "/Memory", ODATA_MEMORY_COLLECTION_TYPE);
    system->memory->name = "Computer System Memory Collection";

    init_memory(system->memory, "DIMM1");
  }
  if (!record_is_exist(odata_id + "/EthernetInterfaces")) {
    log(info) << "[...]System ethernet init";
    system->ethernet = new Collection(odata_id + "/EthernetInterfaces",
                                      ODATA_ETHERNET_INTERFACE_COLLECTION_TYPE);
    system->ethernet->name = "Computer System Ethernet Interface Collection";

    int eth_num =
        improved_stoi(get_popen_string("ifconfig -a | grep eth | wc -l"));
    try {
      log(info) << "eth_num =" << eth_num;
      for (int i = 0; i < eth_num; i++) {
        log(info) << "eth" << i;
        init_ethernet(system->ethernet, to_string(i));
      }

    } catch (const std::exception &) {
      log(info) << "[...]System ethernet init error";
    }
  }

  if (!record_is_exist(odata_id + "/LogServices")) {
    log(info) << "[...]System log service init";
    system->log_service = new Collection(odata_id + "/LogServices",
                                         ODATA_LOG_SERVICE_COLLECTION_TYPE);
    system->log_service->name = "Computer System Log Service Collection";

    init_log_service(system->log_service, "Log1");

    string log_odata = system->log_service->odata.id; // /LogServices까지
    log_odata.append("/Log1/Entries");
    Collection *col = (Collection *)g_record[log_odata];

    string col_odata = col->odata.id;
    LogEntry *entry1 = new LogEntry(col_odata + "/1", "1");
    entry1->entry_type = "Event";
    entry1->name = "Log Entry 2";
    entry1->created = "2022-11-02 13:33:17";
    entry1->severity = "Critical";
    entry1->message.message = "Upper Critical";
    entry1->message.id = "UpperThreshold";
    col->add_member(entry1);

    LogEntry *entry2 = new LogEntry(col_odata + "/2", "2");
    entry2->entry_type = "Event";
    entry2->name = "Log Entry 3";
    entry2->created = "2022-11-02 14:17:44";
    entry2->severity = "OK";
    entry2->message.message = "Status has changed to OK at Resource";
    entry2->message.id = "StatusChangeOK";
    col->add_member(entry2);

    LogEntry *entry3 = new LogEntry(col_odata + "/3", "3");
    entry3->entry_type = "Event";
    entry3->name = "Log Entry 4";
    entry3->created = "2022-11-02 14:22:38";
    entry3->severity = "OK";
    entry3->message.message = "Resource is Updated";
    entry3->message.id = "ResourceUpdated";
    col->add_member(entry3);

  }
  if (!record_is_exist(odata_id + "/SimpleStorage")) {
    log(info) << "[...]System simple storage init";
    system->simple_storage = new Collection(
        odata_id + "/SimpleStorage", ODATA_SIMPLE_STORAGE_COLLECTION_TYPE);
    system->simple_storage->name = "Computer System Simple Storage Collection";

    init_simple_storage(system->simple_storage, "0");
  }
  if (!record_is_exist(odata_id + "/Bios")) {
    log(info) << "[...]System bios init";
    system->bios = new Bios(odata_id + "/Bios", "Bios");

    init_bios(system->bios);
  }
  return;
}

void init_storage_collection(Collection *storage_collection, string _id) {
  string odata_id = storage_collection->odata.id + "/" + _id;

  Storage *storage = new Storage(odata_id, _id);
  storage->id = _id;
  init_storage(storage);

  storage_collection->add_member(storage);
  return;
}

void init_storage(Storage *storage) {
  string odata_id = storage->odata.id;
  /**
   * storage Configuration
   */
  storage->description = "This resource is used to represent a drive for a "
                         "Redfish implementation.";
  storage->status.state = STATUS_STATE_ENABLED;
  storage->status.health = STATUS_HEALTH_OK;

  if (!record_is_exist(odata_id + "/StorageControllers")) {
    storage->controller =
        new List(odata_id + "/StorageControllers", STORAGE_CONTROLLER_TYPE);
    storage->controller->name = "Storage Controllers List";

    int storage_num =
        improved_stoi(get_popen_string("lsblk -o NAME -n | wc -l"));
    vector<string> storage_name = string_split(
        get_popen_string("lsblk -o NAME -n | head -" + to_string(storage_num)),
        '\n');

    for (auto str : storage_name) {
      init_storage_controller(storage->controller, ltrim(str));
    }
  }

  if (!record_is_exist(odata_id + "/Drives")) {
    storage->drives =
        new Collection(odata_id + "/Drives", ODATA_DRIVE_COLLECTION_TYPE);
    storage->drives->name = "Storage Drive Collection";

    init_drive(storage->drives, "0");
  }

  if (!record_is_exist(odata_id + "/Volumes")) {
    storage->volumes =
        new Collection(odata_id + "/Volumes", ODATA_VOLUME_COLLECTION_TYPE);
    storage->volumes->name = "Storage Volume Collection";

    init_volume(storage->volumes, "0");
  }
}

void init_storage_controller(List *storage_controllers_list, string _id) {
  string odata_id = storage_controllers_list->odata.id + "/" + _id;
  string block_dev = "/dev/" + _id;
  StorageControllers *sc = new StorageControllers(odata_id, _id);

  /**
   * @todo : Storage Controllers Configuration
   */

  sc->id = _id;
  // sc->manufacturer = get_popen_string("lsblk -o NAME,VENDOR | grep \"" + _id
  // + "\" | head -1 | awk \'{print $2}\'"); sc->model = get_popen_string("lsblk
  // -o NAME,MODEL | grep \"" + _id + "\" | head -1 | awk \'{print $2}\'");
  // sc->serial_number = get_popen_string("lsblk -o NAME,SERIAL | grep \"" + _id
  // + "\" | head -1 | awk \'{print $2}\'"); sc->part_number =
  // get_popen_string("lsblk -o NAME,PARTUUID | grep \"" + _id + "\" | head -1 |
  // awk \'{print $2}\'");
  sc->manufacturer = get_popen_string(
      "lsblk " + block_dev + " -o NAME,VENDOR -n | awk \'{print $2}\'");
  sc->model = get_popen_string("lsblk " + block_dev +
                               " -o NAME,MODEL -n | awk \'{print $2}\'");
  sc->serial_number = get_popen_string(
      "lsblk " + block_dev + " -o NAME,SERIAL -n | awk \'{print $2}\'");
  sc->part_number = get_popen_string(
      "lsblk " + block_dev + " -o NAME,PARTUUID -n | awk \'{print $2}\'");
  sc->speed_gbps = 0;        //
  sc->firmware_version = ""; //

  sc->identifier.durable_name = get_value_from_cmd_str(
      "fdisk -l | grep \"Disk identifier\"", "Disk identifier");
  sc->identifier.durable_name_format = "UUID";

  sc->support_controller_protocols.push_back("PCIe"); //
  sc->support_device_protocols.push_back("SAS");      //
  sc->support_device_protocols.push_back("SATA");     //

  sc->status.state = STATUS_STATE_ENABLED;
  sc->status.health = STATUS_HEALTH_OK;

  storage_controllers_list->add_member(sc);
  return;
}

void init_drive(Collection *drive_collection, string _id) {
  string odata_id = drive_collection->odata.id + "/" + _id;

  Drive *drive = new Drive(odata_id, _id);

  /**
   * drive Configuration
   */
  drive->id = _id;
  drive->asset_tag = "";
  drive->description = "This resource is used to represent a drive for a "
                       "Redfish implementation.";
  drive->encryption_ability = "";
  drive->encryption_status = "";
  drive->hotspare_type = "";
  drive->manufacturer = "";
  drive->media_type = "";
  drive->model = "";
  drive->name = "";
  drive->sku = "";
  drive->status_indicator = "";
  drive->part_number = "";
  drive->protocol = "";
  drive->revision = "";
  drive->serial_number = "";

  drive->block_size_bytes = 512;
  drive->capable_speed_Gbs = 12;
  drive->negotiated_speed_Gbs = 12;
  drive->predicted_media_life_left_percent = 100;
  drive->rotation_speed_RPM = 10500;

  drive->failure_predicted = false;

  Identifier i;
  i.durable_name = "";
  i.durable_name_format = "UUID";
  drive->identifier.push_back(i);

  drive->physical_location.part_location.service_label = "";
  drive->physical_location.part_location.location_type = "";
  drive->physical_location.part_location.location_ordinal_value = 1;
  drive->physical_location.info = "";
  drive->physical_location.info_format = "SLOT NUMBER";

  drive->status.state = STATUS_STATE_ENABLED;
  drive->status.health = STATUS_HEALTH_OK;

  drive_collection->add_member(drive);
  return;
}

void init_volume(Collection *volume_collection, string _id) {
  string odata_id = volume_collection->odata.id + "/" + _id;

  Volume *volume = new Volume(odata_id, _id);

  /**
   * volume Configuration
   */
  volume->id = _id;
  volume->description = "This resource is used to represent a volume for a "
                        "Redfish implementation.";
  volume->RAID_type = "";
  volume->name = "";
  volume->read_cache_policy = "";
  volume->write_cache_policy = "";
  volume->strip_size_bytes = "";
  volume->display_name = "";
  volume->block_size_bytes = 512;
  volume->capacity_bytes = 238999830528;

  volume->access_capabilities.push_back("Write");
  volume->access_capabilities.push_back("Read");

  volume->status.state = STATUS_STATE_ENABLED;
  volume->status.health = STATUS_HEALTH_OK;

  volume_collection->add_member(volume);
  return;
}

void init_processor(Collection *processor_collection, string _id) {
  string odata_id = processor_collection->odata.id + "/" + _id;

  Processors *processor = new Processors(odata_id, _id);

  /**
   * Processor Configuration
   */
  processor->id = _id;
  processor->name = "Processor";
  processor->socket = _id;
  processor->processor_type = "CPU";
  processor->processor_architecture = get_value_from_cmd_str(
      "lscpu | grep \"Architecture\" | head -1", "Architecture");
  processor->instruction_set = get_popen_string("uname -m");
  processor->manufacturer = get_value_from_cmd_str(
      "lscpu | grep \"Vendor ID\" | head -1", "Vendor ID");
  processor->model = ltrim(string_split(
      get_popen_string("cat /proc/cpuinfo | grep \"model name\" | head -1"),
      ':')[1]);

  processor->max_speed_mhz = improved_stof(get_value_from_cmd_str(
      "lscpu | grep \"BogoMIPS\" | head -1", "BogoMIPS"));
  processor->total_cores = improved_stoi(
      get_value_from_cmd_str("lscpu | grep \"CPU(s)\" | head -1", "CPU(s)"));
  processor->total_threads =
      improved_stoi(get_value_from_cmd_str(
          "lscpu | grep \"Thread(s) per core\" | head -1",
          "Thread(s) per core")) *
      processor->total_cores;

  processor->status.state = STATUS_STATE_ENABLED;
  processor->status.health = STATUS_HEALTH_OK;

  processor->p_id.vendor_id = processor->manufacturer;
  processor->p_id.identification_registers = get_value_from_cmd_str(
      "cat /proc/cpuinfo | grep \"Serial\" | head -1", "Serial");
  processor->p_id.effective_family = get_value_from_cmd_str(
      "cat /proc/cpuinfo | grep \"cpu family\" | head -1", "cpu family");
  processor->p_id.effective_model = get_value_from_cmd_str(
      "cat /proc/cpuinfo | grep \"model\" | grep -v \"model name\" | head -1",
      "model");
  processor->p_id.step = get_value_from_cmd_str(
      "cat /proc/cpuinfo | grep \"stepping\" | head -1", "stepping");
  processor->p_id.microcode_info = get_value_from_cmd_str(
      "cat /proc/cpuinfo | grep \"microcode\" | head -1", "microcode");

  processor_collection->add_member(processor);
  return;
}

void init_memory(Collection *memory_collection, string _id) {
  string odata_id = memory_collection->odata.id + "/" + _id;

  Memory *memory = new Memory(odata_id, _id);

  /**
   * Memory Configuration
   */
  memory->id = _id;
  // memory->rank_count = 2;
  memory->capacity_kib =
      improved_stoi(get_value_from_cmd_str("free -k", "Mem"));
  // memory->data_width_bits = 64;
  // memory->bus_width_bits = 72;
  // memory->error_correction = "MultiBitECC";

  // memory->m_location.socket = 1;
  // memory->m_location.memory_controller = 1;
  // memory->m_location.channel = 1;
  // memory->m_location.slot = 1;

  // memory->memory_type = "DRAM";
  // memory->memory_device_type = "DDR4";
  // memory->base_module_type = "RDIMM";

  // memory->memory_media.push_back("DRAM");
  // memory->max_TDP_milliwatts.push_back(12000);

  memory->status.state = STATUS_STATE_ENABLED;
  memory->status.health = STATUS_HEALTH_OK;

  memory_collection->add_member(memory);
  return;
}

void init_ethernet(Collection *ethernet_collection, string _id) {
  string odata_id = ethernet_collection->odata.id + "/NIC";
  if (_id != "0")
    odata_id += _id;

  EthernetInterfaces *ethernet = new EthernetInterfaces(odata_id, _id);

  /**
   * Ethernet Interface Configuration
   */
  string eth_id = "eth" + _id;
  ethernet->description = "Manager Ethernet Interface";
  ethernet->link_status = "LinkDown";
  ethernet->interfaceEnabled = false;

  // 104번 evb보드ver
  // if (get_popen_string("more /sys/class/net/" + eth_id + "/operstate") ==
  // "up")
  // {
  //     ethernet->link_status = "LinkUp";
  //     ethernet->interfaceEnabled = true;
  // }

  // 109번 렉에 탑재된 cm보드ver
  if (get_popen_string("more /sys/class/net/" + eth_id + "/carrier") == "1") {
    ethernet->link_status = "LinkUp";
    ethernet->interfaceEnabled = true;
  }

  ethernet->permanent_mac_address =
      get_popen_string("more /sys/class/net/" + eth_id + "/address");
  ethernet->mac_address = ethernet->permanent_mac_address;
  ethernet->speed_Mbps = improved_stoi(
      get_popen_string("more /sys/class/net/" + eth_id + "/speed"));
  ethernet->autoneg = true; // it can be set false. but not recommended. it sets
                            // speed and duplex automatically
  ethernet->full_duplex = false;
  if (get_popen_string("more /sys/class/net/" + eth_id + "/duplex") == "full")
    ethernet->full_duplex = true;
  ethernet->mtu_size =
      improved_stoi(get_popen_string("more /sys/class/net/" + eth_id + "/mtu"));
  ethernet->hostname = get_popen_string("more /etc/hostname");
  ethernet->fqdn = get_popen_string("hostname -f");

  // for nameserver
  vector<string> resolv_data;
  string back = get_popen_string("more /etc/resolv.conf");
  if (back.length() > 1) {
    resolv_data = string_split(back, ' ');
    ethernet->name_servers.push_back(resolv_data[1]);
    ethernet->name_servers.push_back(resolv_data[2]);
  }
  if (ethernet->name_servers.size() == 2) {
    ethernet->name_servers.push_back("::ffff:808:404");
    ethernet->name_servers.push_back("::ffff:808:808");
  }
  // ethernet->name_servers = string_split(get_popen_string("more
  // /etc/resolv.conf"), ' ');
  ethernet->ipv6_default_gateway = string_split(
      string_split(get_popen_string("ip -6 route | head -1"), ' ')[0], '/')[0];

  if (fs::exists(DHCPV4_CONF)) {
    log(warning) << "NOT IMPLEMENTED : read dhcpv4 conf";
  }
  if (fs::exists(DHCPV6_CONF)) {
    log(warning) << "NOT IMPLEMENTED : read dhcpv6 conf";
  }

  if (ethernet->link_status == "LinkUp") {
    int ipv4_num =
        improved_stoi(get_popen_string("ifconfig -a | grep eth1 | wc -l"));
        // improved_stoi(get_popen_string("ifconfig -a | grep eth0 | wc -l")); // 104번 보드는 eth0에 주소할당인데 109번 보내는
        // CM은 eth1에 주소할당이라 주소 못읽어옴

    for (int i = 0; i < ipv4_num; i++) {
      string ipv4_alias = eth_id;

      cout << "eth_id / ipv4alias : " << eth_id << " / " << ipv4_alias << endl;
      if (i != 0)
        ipv4_alias += ":" + i;

      IPv4_Address ipv4;
      ipv4.address = get_value_from_cmd_str(
          "ifconfig " + ipv4_alias + " | grep \"inet addr\"", "inet addr");
      ipv4.address_origin = get_value_from_cmd_str(
          "cat /etc/network/interfaces | grep \"iface " + ipv4_alias + "\"",
          "inet");
      ipv4.subnet_mask = get_value_from_cmd_str(
          "ifconfig " + ipv4_alias + " | grep \"inet addr\"", "Mask");
      ipv4.gateway =
          string_split(get_popen_string("ip r | grep default"), ' ')[2];
      ethernet->v_ipv4.push_back(ipv4);

      IPv6_Address ipv6;
      string ipv6_temp = get_value_from_cmd_str(
          "ifconfig " + ipv4_alias + " | grep \"inet6 addr\"", "inet6 addr");
      ipv6.address = string_split(ipv6_temp, '/')[0];
      ipv6.prefix_length = improved_stoi(string_split(ipv6_temp, '/')[1]);
      ipv6.address_origin = "DHCPv6";
      log(debug) << ipv6.address;
      log(debug) << ipv6.prefix_length;
      // ipv6.address_state
      ethernet->v_ipv6.push_back(ipv6);
    }

    if (fs::exists(VLAN_CONF)) {
      if (improved_stoi(get_popen_string("cat /proc/net/vlan/config | grep " +
                                         eth_id + " | wc -l"))) {
        Vlan v;
        v.vlan_enable = true;
        v.vlan_id = improved_stoi(string_split(
            get_popen_string("cat /proc/net/vlan/config | grep " + eth_id),
            '|')[1]);
      }
    }
  }

  ethernet->status.state = STATUS_STATE_ENABLED;
  ethernet->status.health = STATUS_HEALTH_OK;

  ethernet_collection->add_member(ethernet);

  // char domain_name_server[100]={0};
  // get_ddns_domain_name(domain_name_server);
  // this->fqdn = string((char *)domain_name_server);
  // this->id = _ethernet_id;
  // this->name = _ethernet_id;
  // this->description = "";
  // this->hostname = "";
  // g_record[_odata_id] = this;
  // this->index = _index;
  // std::vector<uint8_t> iptemp = ipmiNetwork[index].mac_addr;
  // macaddress.resize(30);
  // sprintf((char *)macaddress.c_str(), "%02x.%02x.%02x.%02x.%02x.%02x\0",
  // iptemp.at(0), iptemp.at(1), iptemp.at(3), iptemp.at(4), iptemp.at(5));
  // macaddress.shrink_to_fit();
  // macaddress.resize(macaddress.find('\u0000'));
  // //cout << "Ethernet macc_addr = " << macaddress << endl;

  // char cmd_str[128] = {0};
  // memset(cmd_str, 0, sizeof(char) * 128);
  // mtusize.resize(30);
  // sprintf(cmd_str, "cat /sys/class/net/eth%d/mtu", index);
  // strcpy((char *)mtusize.c_str(), get_popen_string(cmd_str));
  // mtusize.resize(mtusize.find('\u0000'));
  // string temp;
  // char ttemp[256] = {0};
  // memset(cmd_str, 0, sizeof(char) * 128);
  // sprintf(cmd_str, "cat /sys/class/net/eth%d/operstate", index);
  // strcpy(ttemp, get_popen_string(cmd_str));
  // //cout << "ttemp= " << ttemp << endl;

  // if (strncmp(ttemp, "up", 2) == 0)
  // {
  //     linkstatus = "LinkUp";
  // }
  // else if (strncmp(ttemp, "down", 4) == 0)
  // {
  //     linkstatus = "LinkDown";
  // }
  // else
  // {
  //     linkstatus = "NoLink";
  // }
  // memset(cmd_str, 0, sizeof(char) * 128);
  // temp.resize(30);
  // sprintf(cmd_str, "cat /sys/class/net/eth%d/duplex", index);
  // strcpy((char *)temp.c_str(), get_popen_string(cmd_str));
  // temp.resize(temp.find('\u0000'));
  // if (temp == "full")
  //     FullDuplex = true;
  // else
  //     FullDuplex = false;

  // this->hostname = string(ipmiApplication.g_host_name);
  // iptemp=ipmiNetwork[index].ip_addr;
  // address.resize(30);
  // sprintf((char *)address.c_str(), "%d.%d.%d.%d\0", iptemp.at(0),
  // iptemp.at(1), iptemp.at(2), iptemp.at(3));
  // address.resize(address.find('\u0000'));
  // //cout<<"Ethernet addr = "<<address<<endl;

  // iptemp=ipmiNetwork[index].net_mask;
  // subnetMask.resize(30);
  // sprintf((char *)subnetMask.c_str(), "%d.%d.%d.%d\0", iptemp.at(0),
  // iptemp.at(1), iptemp.at(2), iptemp.at(3));
  // subnetMask.resize(subnetMask.find('\u0000'));

  // //cout<<"Ethernet SubnetMask = "<<subnetMask<<endl;

  // iptemp=ipmiNetwork[index].df_gw_ip_addr;
  // gateway.resize(30);
  // sprintf((char *)gateway.c_str(), "%d.%d.%d.%d\0", iptemp.at(0),
  // iptemp.at(1), iptemp.at(2), iptemp.at(3));
  // gateway.resize(gateway.find('\u0000'));

  // //cout<<"Ethernet Gateway = "<<gateway<<endl;
  // if (ipmiNetwork[index].ip_src ==1)
  //     this->addressOrigin="Static";
  // else if(ipmiNetwork[index].ip_src ==1)
  //     this->addressOrigin="DHCP";
  // else
  //     this->addressOrigin="Unspecified";

  // iptemp=ipmiNetwork[index].ip_addr_v6;
  // addressOriginv6.resize(60);
  // for (int i = 0; i < iptemp.size(); i++)
  // {
  //     this->addressOriginv6 += (this->addressOriginv6.at(i));
  // }
  // addressOriginv6.resize(addressOriginv6.find('\u0000'));

  // this->addressOriginv6 = "DHCPv6";

  // iptemp = ipmiNetwork[index].df_gw_ip_addr_v6;
  // gatewayv6.resize(60);
  // for (int i = 0; i < iptemp.size(); i++)
  // {
  //     this->gatewayv6 += (this->gatewayv6.at(i));
  // }
  // gatewayv6.resize(gatewayv6.find('\u0000'));

  // iptemp = ipmiNetwork[index].net_mask_v6;
  // this->prefixLength.resize(60);
  // for (int i = 0; i < iptemp.size(); i++)
  // {
  //     this->prefixLength += (this->prefixLength.at(i));
  // }
  // prefixLength.resize(prefixLength.find('\u0000'));
  // this->vlanid=(int)ipmiNetwork[index].vlan_id;

  // if(ipmiNetwork[index].vlan_enable==0)
  //    this->vlan_enable=false;
  // else
  //     this->vlan_enable=true;

  return;
}

void init_log_service(Collection *log_service_collection, string _id) {
  string odata_id = log_service_collection->odata.id + "/" + _id;

  LogService *log_service = new LogService(odata_id, _id);
  log_service->max_number_of_records = 1000;
  log_service->overwrite_policy = "WrapsWhenFull";
  log_service->datetime_local_offset = "+09:00";
  log_service->service_enabled = true;

  log_service->status.state = STATUS_STATE_ENABLED;
  log_service->status.health = STATUS_HEALTH_OK;

  if (!record_is_exist(odata_id + "/Entries")) {
    log_service->entry =
        new Collection(odata_id + "/Entries", ODATA_LOG_ENTRY_COLLECTION_TYPE);
    log_service->entry->name = "Computer System Log Entry Collection";

    init_log_entry(log_service->entry, "0");
  }

  SyslogFilter temp;
  temp.logFacilities.push_back("Syslog");
  temp.lowestSeverity = "All";
  log_service->syslogFilters.push_back(temp);

  log_service_collection->add_member(log_service);
  return;
}

void init_log_entry(Collection *log_entry_collection, string _id) {
  string odata_id = log_entry_collection->odata.id + "/" + _id;

  LogEntry *log_entry = new LogEntry(odata_id, _id);

  log_entry->entry_type = "Event";
  log_entry->name = "Log Entry 1";
  log_entry->created = "2022-11-02 13:07:05";//currentDateTime();//"2021-07-25.13:07:10"; 뭐여이거 하드코딩
  log_entry->severity = "OK";
  log_entry->message.message = "Current temperature";
  log_entry->message.id = "Keti.1.0.TempReport";
  log_entry->message.message_args.push_back("30");
  log_entry->sensor_number = 4;

  log_entry_collection->add_member(log_entry);
  return;
}

void init_simple_storage(Collection *simple_storage_collection, string _id) {
  string odata_id = simple_storage_collection->odata.id + "/" + _id;

  SimpleStorage *simple_storage = new SimpleStorage(odata_id, _id);

  /**
   * @todo SimpleStorage Configuration
   */
  // simple_storage->description = "System SATA";
  // simple_storage->uefi_device_path = "Acpi(PNP0A03, 0) / Pci(1F|1) /
  // Ata(Primary,Master) / HD(Part3, Sig00110011)";
  vector<string> vec =
      string_split(get_popen_string("lsblk | grep disk"), '\n');

  for (auto str : vec) {
    Device_Info info;
    vector<string> info_vec = string_split(str, ' ');

    if (info_vec[3].back() == 'G') {
      info_vec[3].pop_back();
      info.capacity_KBytes = improved_stoi(info_vec[3]) * 1024 * 1024;
    } else if (info_vec[3].back() == 'M') {
      info_vec[3].pop_back();
      info.capacity_KBytes = improved_stoi(info_vec[3]) * 1024;
    } else if (info_vec[3].back() == 'K') {
      info_vec[3].pop_back();
      info.capacity_KBytes = improved_stoi(info_vec[3]);
    } else {
      log(warning) << "disk size is abnormal..";
    }

    info.name = info_vec[0];
    string block_dev = "/dev/" + info.name;
    // info.manufacturer = get_popen_string("lsblk -o NAME,VENDOR | grep " +
    // info_vec[0] + " | head -1 | awk \'{print $2}\'"); info.model =
    // get_popen_string("lsblk -o NAME,MODEL | grep " + info_vec[0] + " | head
    // -1 | awk \'{print $2}\'");
    info.manufacturer = get_popen_string(
        "lsblk " + block_dev + " -o NAME,VENDOR -n | awk \'{print $2}\'");
    info.model = get_popen_string("lsblk " + block_dev +
                                  " -o NAME,MODEL -n | awk \'{print $2}\'");
    info.status.state = STATUS_STATE_ENABLED;
    info.status.health = STATUS_HEALTH_OK;

    simple_storage->devices.push_back(info);
  }

  simple_storage->status.state = STATUS_STATE_ENABLED;
  simple_storage->status.health = STATUS_HEALTH_OK;

  simple_storage_collection->add_member(simple_storage);

  return;
}

void init_bios(Bios *bios) {
  /**
   * @todo 여기에 bios 일반멤버변수값 넣어주기
   */
  bios->id = "BIOS";
  bios->name = "BIOS Configuration Current Settings";
  bios->attribute_registry = "Attribute registry";
  bios->attribute.boot_mode = "Uefi";
  bios->attribute.embedded_sata = "Raid";
  bios->attribute.nic_boot1 = "NetworkBoot";
  bios->attribute.nic_boot2 = "Disabled";
  bios->attribute.power_profile = "MaxPerf";
  bios->attribute.proc_core_disable = 0;
  bios->attribute.proc_hyper_threading = "Enabled";
  bios->attribute.proc_turbo_mode = "Enabled";
  bios->attribute.usb_control = "UsbEnabled";

  return;
}

void init_chassis(Chassis *chassis, string _id) {
  string odata_id = chassis->odata.id;
  chassis->fru_this = &fru_rec.find(0)->second.find(0)->second;

  /**
   * @todo 여기에 chassis 일반멤버변수값 넣어주기
   */
  chassis->id = _id;
  chassis->chassis_type = "Enclosure"; // fru_this->fru_chassis.type ;
  chassis->manufacturer = string((char *)(chassis->fru_this->fru_board.mfg));
  chassis->model = "";
  chassis->serial_number = string((char *)chassis->fru_this->fru_board.serial);
  chassis->part_number =
      string((char *)chassis->fru_this->fru_chassis.part_number);
  chassis->asset_tag = string((char *)chassis->fru_this->product.asset_tag);
  chassis->power_state = POWER_STATE_ON;

  chassis->indicator_led = LED_OFF;
  // chassis->led_off(LED_YELLOW);
  // chassis->led_off(LED_RED);
  // chassis->led_blinking(LED_GREEN);

  chassis->status.state = STATUS_STATE_ENABLED;
  chassis->status.health = STATUS_HEALTH_OK;

  chassis->location.postal_address.country = "";
  chassis->location.postal_address.territory = "";
  chassis->location.postal_address.city = "";
  chassis->location.postal_address.street = "";
  chassis->location.postal_address.house_number = "";
  chassis->location.postal_address.name = "";
  chassis->location.postal_address.postal_code = "";

  chassis->location.placement.row = "";
  chassis->location.placement.rack = "";
  chassis->location.placement.rack_offset_units = "";
  chassis->location.placement.rack_offset = 0;

  if (!record_is_exist(odata_id + "/Sensors")) {
    chassis->sensors =
        new Collection(odata_id + "/Sensors", ODATA_SENSOR_COLLECTION_TYPE);
    chassis->sensors->name = "Computer Sensor Collection";

    init_sensor(chassis->sensors, "CabinetTemp");
  }
  if (!record_is_exist(odata_id + "/Thermal")) {
    chassis->thermal = new Thermal(odata_id + "/Thermal");
    chassis->thermal->name = "CMM Chassis Thermal";

    init_thermal(chassis->thermal);
  }
  if (!record_is_exist(odata_id + "/Storage")) {
    chassis->storage =
        new Collection(odata_id + "/Storage", ODATA_STORAGE_COLLECTION_TYPE);
    chassis->storage->name = "Chassis Storage Collection";

    init_storage_collection(chassis->storage, "1");

    // chassis->storage = new Storage(odata_id + "/Storage");
    // chassis->storage->name = "CMM Chassis Storage";
    // chassis->storage->id = "/Storage";
    // init_storage(chassis->storage);
  }
  if (!record_is_exist(odata_id + "/Power")) {
    chassis->power = new Power(odata_id + "/Power");
    chassis->power->name = "CMM Chassis Power";

    init_power(chassis->power);
  }
  if (!record_is_exist(odata_id + "/LogServices")) {
    chassis->log_service = new Collection(odata_id + "/LogServices",
                                          ODATA_LOG_SERVICE_COLLECTION_TYPE);
    chassis->log_service->name = "Chassis Log Service Collection";

    init_log_service(chassis->log_service, "Log1");

    string log_odata = chassis->log_service->odata.id; // /LogServices까지
    log_odata.append("/Log1/Entries");
    Collection *col = (Collection *)g_record[log_odata];

    string col_odata = col->odata.id;
    LogEntry *entry1 = new LogEntry(col_odata + "/1", "1");
    entry1->entry_type = "SEL";
    entry1->name = "Log Entry 2";
    entry1->created = "2022-11-02 13:08:05";
    entry1->severity = "Critical";
    entry1->message.message = "Upper Critical";
    entry1->message.id = "SEL Message";
    col->add_member(entry1);

    LogEntry *entry2 = new LogEntry(col_odata + "/2", "2");
    entry2->entry_type = "Event";
    entry2->name = "Log Entry 3";
    entry2->created = "2022-11-02 13:42:29";
    entry2->severity = "Critical";
    entry2->message.message = "Status has changed to Critical at Resource";
    entry2->message.id = "StatusChangeCritical";
    col->add_member(entry2);

    LogEntry *entry3 = new LogEntry(col_odata + "/3", "3");
    entry3->entry_type = "Event";
    entry3->name = "Log Entry 4";
    entry3->created = "2022-11-02 15:31:02";
    entry3->severity = "OK";
    entry3->message.message = "Resource is Removed";
    entry3->message.id = "ResourceRemoved";
    col->add_member(entry3);

    LogEntry *entry4 = new LogEntry(col_odata + "/4", "4");
    entry4->entry_type = "SEL";
    entry4->name = "Log Entry 5";
    entry4->created = "2022-11-02 16:55:27";
    entry4->severity = "Critical";
    entry4->message.message = "Lower Critical";
    entry4->message.id = "SEL Message";
    col->add_member(entry4);
  }

  return;
}

void init_sensor(Collection *sensor_collection, string _id) {
  string odata_id = sensor_collection->odata.id + "/" + _id;
  Sensor *sensor;

  if (record_is_exist(odata_id))
    return;

  sensor = new Sensor(odata_id, _id);
  sensor_collection->add_member(sensor);
  return;
}

void init_thermal(Thermal *thermal) {
  string odata_id = thermal->odata.id;

  /**
   * @todo 여기에 thermal 일반멤버변수값 넣어주기
   */

  if (!record_is_exist(odata_id + "/Temperatures")) {
    thermal->temperatures =
        new List(odata_id + "/Temperatures", TEMPERATURE_TYPE);
    thermal->temperatures->name = "Chassis Temperatures";

    init_temperature(thermal->temperatures, "0");
    init_temperature(thermal->temperatures, "1");
  }

  if (!record_is_exist(odata_id + "/Fans")) {
    thermal->fans = new List(odata_id + "/Fans", FAN_TYPE);
    thermal->fans->name = "Chassis Fans";

    init_fan(thermal->fans, "0");
    init_fan(thermal->fans, "1");
  }
  return;
}

void init_temperature(List *temperatures_list, string _id) {
  string odata_id = temperatures_list->odata.id + "/" + _id;

  Temperature *temper = new Temperature(odata_id, _id);

  /**
   * Temperature Configuration
   */
  // double temp[2] = {0};
  //     if (get_intake_temperature_config(temp)) {
  //         log(info) << "Chassis temperature min value = " << temp[0];
  //         log(info) << "Chassis temperature max value = " << temp[1];
  //     }
  //     // 이거 순서 바뀐거같은데 temp[0]이 maxvalue인듯

  //     for (uint8_t i = 0; i < 4; i++)
  //     {
  //         /**
  //          * @todo 여기에 temperatures 일반멤버변수값 넣어주기
  //          */
  //         ostringstream s;
  //         s << thermal->temperatures->odata.id << "/" << to_string(i);
  //         Temperature *intake_temperature = new Temperature(s.str(),
  //         to_string(i)); intake_temperature->name = "Chassis Intake
  //         Temperature"; intake_temperature->physical_context = "Intake";
  //         intake_temperature->min_reading_range_temp = temp[0];
  //         intake_temperature->max_reading_range_temp = temp[1];
  //         intake_temperature->upper_threshold_non_critical = round(temp[1] *
  //         0.6); intake_temperature->upper_threshold_critical = round(temp[1]
  //         * 0.7); intake_temperature->upper_threshold_fatal = round(temp[1] *
  //         0.85); intake_temperature->read(i, INTAKE_CONTEXT);
  //         intake_temperature->sensor_num = i;
  //         thermal->temperatures->add_member(intake_temperature);
  //     }

  //     ostringstream s;
  //     s << thermal->temperatures->odata.id << "/" <<
  //     to_string(thermal->temperatures->members.size()); Temperature
  //     *cpu_temperature = new Temperature(s.str(),
  //     to_string(thermal->temperatures->members.size()));
  //     cpu_temperature->name = "Chassis Manager CPU Temperature";
  //     cpu_temperature->physical_context = "CPU";
  //     cpu_temperature->min_reading_range_temp = 0;
  //     cpu_temperature->max_reading_range_temp = 100;
  //     cpu_temperature->upper_threshold_non_critical =
  //     round(cpu_temperature->max_reading_range_temp * 0.7);
  //     cpu_temperature->upper_threshold_critical =
  //     round(cpu_temperature->max_reading_range_temp * 0.75);
  //     cpu_temperature->upper_threshold_fatal =
  //     round(cpu_temperature->max_reading_range_temp * 0.8);
  //     cpu_temperature->read(thermal->temperatures->members.size(),
  //     CPU_CONTEXT); cpu_temperature->sensor_num =
  //     thermal->temperatures->members.size();
  //     thermal->temperatures->add_member(cpu_temperature);

  // for fanmode test
  if (_id == "0") {
    temper->reading_celsius = 50;
    temper->sensor_num = 0;
  } else if (_id == "1") {
    temper->reading_celsius = 30;
    temper->sensor_num = 1;
  }

  temperatures_list->add_member(temper);
  return;
}

void init_fan(List *fans_list, string _id) {
  string odata_id = fans_list->odata.id + "/" + _id;

  Fan *fan = new Fan(odata_id, _id);

  /**
   * Fan Configuration
   */

  // for(int i=0; i<2; i++)
  // {
  //     ostringstream os;
  //     os << thermal->fans->odata.id << "/" << to_string(i);// << "0";
  //     Fan *chassis_f = new Fan(os.str(), to_string(i));
  //     chassis_f->max_reading_range = 3000 * (i+1);
  //     chassis_f->sensor_num = i;
  //     thermal->fans->add_member(chassis_f);
  // }

  // for fanmode test
  if (_id == "0") {
    fan->max_reading_range = 6000;
    fan->sensor_num = 0;
  } else if (_id == "1") {
    fan->max_reading_range = 3500;
    fan->sensor_num = 1;
  }

  fans_list->add_member(fan);
  return;
}

void init_power(Power *power) {
  string odata_id = power->odata.id;
  ostringstream os;

  /**
   * @todo 여기에 power 일반멤버변수값 넣어주기
   */

  if (!record_is_exist(odata_id + "/PowerControl")) {
    power->power_control =
        new List(odata_id + "/PowerControl", POWER_CONTROL_TYPE);
    power->power_control->name = "Chassis PowerControl";

    init_power_control(power->power_control, "0");
  }
  if (!record_is_exist(odata_id + "/Voltages")) {
    power->voltages = new List(odata_id + "/Voltages", VOLTAGE_TYPE);
    power->voltages->name = "Chassis Voltages";

    init_voltage(power->voltages, "0");
  }
  if (!record_is_exist(odata_id + "/PowerSupplies")) {
    power->power_supplies =
        new List(odata_id + "/PowerSupplies", POWER_SUPPLY_TYPE);
    power->power_supplies->name = "Chassis PowerSupplies";

    init_power_supply(power->power_supplies, "0");
  }
  return;
}

void init_power_control(List *power_control_list, string _id) {
  string odata_id = power_control_list->odata.id + "/" + _id;

  PowerControl *pc = new PowerControl(odata_id, _id);

  /**
   * Power Control Configuration
   */
  // os.str("");
  // os << power->power_control->odata.id << "/0";
  // PowerControl *chassis_pc = new PowerControl(os.str(), "0~~");
  // power->power_control->add_member(chassis_pc);

  power_control_list->add_member(pc);
  return;
}

void init_voltage(List *voltages_list, string _id) {
  string odata_id = voltages_list->odata.id + "/" + _id;

  Voltage *v = new Voltage(odata_id, _id);

  /**
   * Voltage Configuration
   */
  // os.str("");
  // os << power->voltages->odata.id << "/0";
  // Voltage *chassis_volt = new Voltage(os.str(), "0~~");
  // power->voltages->add_member(chassis_volt);

  voltages_list->add_member(v);
  return;
}

void init_power_supply(List *power_supplies_list, string _id) {
  string odata_id = power_supplies_list->odata.id + "/" + _id;

  PowerSupply *ps = new PowerSupply(odata_id, _id);

  /**
   * PowerSupply Configuration
   */
  // os.str("");
  // os << power->power_supplies->odata.id << "/0";
  // PowerSupply *chassis_ps = new PowerSupply(os.str(), "0~~");
  // power->power_supplies->add_member(chassis_ps);

  power_supplies_list->add_member(ps);
  return;
}

void init_manager(Manager *manager, string _id) {
  string odata_id = manager->odata.id;
  manager->fru_this = &fru_rec.find(0)->second.find(0)->second;
  /**
   * Manager Configuration
   */
  manager->name = "CMM Manager";
  manager->manager_type = "EnclosureManager";
  manager->firmware_version = "v1";
  manager->uuid = string(uuid_str);
  manager->model = get_popen_string("uname -m");
  manager->datetime = currentDateTime();
  manager->datetime_offset = "+09:00";
  manager->power_state = "";
  manager->description = "";

  KETI_define::global_enabler = 8;
  manager->status.state = STATUS_STATE_ENABLED;
  manager->status.health = STATUS_HEALTH_OK;

  // this->id=_managers_id;
  // this->type = _managers_id;
  // this->firmwareversion =REDFISH_VERSION;
  // this->name= "bmc";

  // string odata=_odata_id;
  // odata=odata+"/NetworkProtocol";
  // status.state="Enabled";
  // status.health="OK";

  // network=new NetworkProtocl(odata,"NetworkProtocol");
  // odata=_odata_id;
  // odata=odata+"/EthernetInterfaces";
  // ethernet= new Collection(odata,ODATA_ETHERNET_INTERFACE_COLLECTION_TYPE);
  // ethmember= (sizeof(ipmiNetwork)/sizeof(Ipminetwork));

  // for(int i =0; i<ethmember;i++)
  // {
  //     string oodata=odata+"/eth"+to_string(i);
  //     Ethernet *member=new Ethernet(oodata,ODATA_ETHERNET_INTERFACE_TYPE,i);

  //     ethernet->add_member(member);
  // }

  // this->datetime=currentDateTime();

  // g_record[_odata_id] = this;

  if (!record_is_exist(odata_id + "/NetworkProtocol")){
      manager->network = new NetworkProtocol(odata_id + "/NetworkProtocol", "NetwrokProtocol");
      manager->network->name = "CMM Network Protocol Config";

      init_network_protocol(manager->network);
  }


  if (!record_is_exist(odata_id + "/EthernetInterfaces")) {
    manager->ethernet =
        new Collection(odata_id + "/EthernetInterfaces",
                       ODATA_ETHERNET_INTERFACE_COLLECTION_TYPE);
    manager->ethernet->name = "Manager Ethernet Interface Collection";

    int eth_num =
        improved_stoi(get_popen_string("ifconfig -a | grep eth | wc -l"));
    for (int i = 0; i < eth_num; i++) {
      init_ethernet(manager->ethernet, to_string(i));
    }
  }

  if (!record_is_exist(odata_id + "/LogServices")) {
    manager->log_service = new Collection(odata_id + "/LogServices",
                                          ODATA_LOG_SERVICE_COLLECTION_TYPE);
    manager->log_service->name = "Manager Log Service Collection";

    init_log_service(manager->log_service, "Log1");

    string log_odata = manager->log_service->odata.id; // /LogServices까지
    log_odata.append("/Log1/Entries");
    Collection *col = (Collection *)g_record[log_odata];

    string col_odata = col->odata.id;
    LogEntry *entry1 = new LogEntry(col_odata + "/1", "1");
    entry1->entry_type = "Event";
    entry1->name = "Log Entry 2";
    entry1->created = "2022-11-02 16:07:46";
    entry1->severity = "Warning";
    entry1->message.message = "Status has changed to Warning at Resource";
    entry1->message.id = "StatusChangeWarning";
    col->add_member(entry1);

    LogEntry *entry2 = new LogEntry(col_odata + "/2", "2");
    entry2->entry_type = "Event";
    entry2->name = "Log Entry 3";
    entry2->created = "2022-11-02 16:19:30";
    entry2->severity = "OK";
    entry2->message.message = "Resource is Created";
    entry2->message.id = "ResourceCreated";
    col->add_member(entry2);
  }
  if (!record_is_exist(odata_id + "/VirtualMedia")) {
    manager->virtual_media = new Collection(
        odata_id + "/VirtualMedia", ODATA_VIRTUAL_MEDIA_COLLECTION_TYPE);
    manager->virtual_media->name = "VirtualMediaCollection";
  }

  if (!record_is_exist(odata_id + "/Radius")) {
    manager->radius = new Radius(odata_id + "/Radius");
    manager->radius->name = "Radius";

    init_radius(manager->radius);
  }

  return;
}

void snmp_config_init(Snmp *snmp)
{
  snmp->protocol_enabled = false;

  snmp->enable_SNMPv1 = false;
  snmp->enable_SNMPv2c = false;
  snmp->enable_SNMPv3 = false;
  
  snmp->port = DEFAULT_SNMP_PORT;
  
  snmp->hide_community_strings = false;
  
  // vector swap trick을 통한 community string 벡터 초기화
  snmp->community_strings.clear();
  vector<Community_String>().swap(snmp->community_strings);

  snmp->community_access_mode = "Limited";

  // ==== SNMPv3 ====
  snmp->authentication_protocol = "CommunityString";
  snmp->encryption_protocol = "None";
  
  snmp->engine_id.architectureId = "";
  snmp->engine_id.enterpriseSpecificMethod = "";
  snmp->engine_id.privateEnterpriseId = "";
}

void ntp_config_init(NTP *ntp)
{
  ntp->protocol_enabled = false;

  // use NTP
  ntp->port = DEFAULT_NTP_PORT;
  string getntpcmd = "cat /etc/ntp.conf | grep server | awk {\'print $2\'}";
  ntp->ntp_servers = string_split(get_popen_string(getntpcmd), '\n');
  ntp->primary_ntp_server = "";
  ntp->secondary_ntp_server = "";

  // not use NTP
  ntp->date_str = "";
  ntp->time_str = "";
  ntp->timezone = "";
}

void init_network_protocol(NetworkProtocol *network)
{
  /**
     * Network Protocol Configuration
     */
    network->hostname = get_popen_string("cat /etc/hostname");
    network->description = "Manager Network Service";
    network->fqdn = get_popen_string("hostname -f");
    
    // cmm doesn't use ipmi 
    network->ipmi_enabled = false;
    network->ipmi_port = DEFAULT_IPMI_PORT;
    
    network->kvmip_enabled = true;
    network->kvmip_port = DEFAULT_KVMIP_PORT;
    
    network->https_enabled = true;
    network->https_port = DEFAULT_HTTPS_PORT;
    
    network->http_enabled = true;
    network->http_port = DEFAULT_HTTP_PORT;
    
    network->virtual_media_enabled = true;
    network->virtual_media_port = DEFAULT_VIRTUAL_MEDIA_PORT;
    
    network->ssh_enabled = true;
    network->ssh_port = DEFAULT_SSH_PORT;
    
    network->status.state = STATUS_STATE_ENABLED;
    network->status.health = STATUS_HEALTH_OK;

    snmp_config_init(&(network->snmp));

    ntp_config_init(&(network->ntp));

    // if(!fs::exists("/etc/iptables.rules"))
    system("iptables -F");
    init_iptable(network);
}

void insert_virtual_media(Collection *virtual_media_collection, string _id) {
  string odata_id = virtual_media_collection->odata.id + "/" + _id;
  VirtualMedia *virtual_media;

  if (record_is_exist(odata_id))
    return;

  virtual_media = new VirtualMedia(odata_id);

  /**
   * @todo 여기에 virtual_media 일반멤버변수값 넣어주기
   */
  virtual_media->id = _id;
  virtual_media->name = "VirtualMedia";
  virtual_media->image = "http://192.168.1.2/Core-current.iso";
  virtual_media->image_name = "Core-current.iso";
  virtual_media->media_type.push_back("CD");
  virtual_media->media_type.push_back("DVD");
  virtual_media->connected_via = "URI";
  virtual_media->inserted = true;
  virtual_media->write_protected = true;
  virtual_media->user_name = "test";
  virtual_media->password = "password";

  virtual_media_collection->add_member(virtual_media);
  return;
}

void init_radius(Radius *radius) {
  radius->radius_server = "localhost";
  radius->radius_secret = "SECRET";
  radius->radius_port = DEFAULT_RADIUS_PORT;
  radius->radius_enabled = false;

  return;
}

void init_update_service(UpdateService *update_service) {
  string odata_id = update_service->odata.id;

  /**
   * Update Service Configuration
   */
  update_service->id = "";
  update_service->service_enabled = true;
  update_service->http_push_uri = "";

  update_service->status.state = STATUS_STATE_ENABLED;
  update_service->status.health = STATUS_HEALTH_OK;

  if (!record_is_exist(odata_id + "/FirmwareInventory")) {
    update_service->firmware_inventory =
        new Collection(odata_id + "/FirmwareInventory",
                       ODATA_SOFTWARE_INVENTORY_COLLECTION_TYPE);
    update_service->firmware_inventory->name = "Firmware Inventory Collection";

    string cm_name = MODULE_NAME;
    // init_software_inventory(update_service->firmware_inventory, "CMM");
    SoftwareInventory *firm_cm1 = init_software_inventory(update_service->firmware_inventory, cm_name);
    // SoftwareInventory *firm_cm1 = init_software_inventory(update_service->firmware_inventory, "CM1");
    firm_cm1->version = "v3.0";
    firm_cm1->manufacturer = "KETI_BMC";
    firm_cm1->release_date = currentDateTime();
    firm_cm1->lowest_supported_version = "v2.0";
    // SoftwareInventory *cmm =
    //     init_software_inventory(update_service->firmware_inventory, "CMM");
    // cmm->version = "v1.0";
    // cmm->manufacturer = "KETI";
    // cmm->release_date = currentDateTime();
    // cmm->lowest_supported_version = "v1.0";

    // SoftwareInventory *web =
    //     init_software_inventory(update_service->firmware_inventory, "WEB");
    // web->version = "v1.0";
    // web->manufacturer = "KETI";
    // web->release_date = currentDateTime();
    // web->lowest_supported_version = "v1.0";

    // SoftwareInventory *ha =
    //     init_software_inventory(update_service->firmware_inventory, "HA");
    // ha->version = "v1.0";
    // ha->manufacturer = "KETI";
    // ha->release_date = currentDateTime();
    // ha->lowest_supported_version = "v1.0";
  }

  if (!record_is_exist(odata_id + "/SoftwareInventory")) {
    update_service->software_inventory = new Collection(odata_id + "/SoftwareInventory",
                       ODATA_SOFTWARE_INVENTORY_COLLECTION_TYPE);
    update_service->software_inventory->name = "Software Inventory Collection";

    // init_software_inventory(update_service->software_inventory, "CMM");
    string cm_name = MODULE_NAME;
    SoftwareInventory *soft_cm1 = init_software_inventory(update_service->software_inventory, cm_name);
    // SoftwareInventory *soft_cm1 = init_software_inventory(update_service->software_inventory, "CM1");
    soft_cm1->version = "v2.0";
    soft_cm1->manufacturer = "KETI_BMC_Ibmc";
    soft_cm1->release_date = currentDateTime();
    soft_cm1->lowest_supported_version = "v2.0";

    cm_name = MODULE_NAME;
    SoftwareInventory *soft_cm1_reading = init_software_inventory(update_service->software_inventory, cm_name.append("-READING"));
    // SoftwareInventory *soft_cm1_reading = init_software_inventory(update_service->software_inventory, "CM1-READING");
    soft_cm1_reading->version = "v1.0";
    soft_cm1_reading->manufacturer = "KETI_BMC_READING";
    soft_cm1_reading->release_date = currentDateTime();
    soft_cm1_reading->lowest_supported_version = "v1.0";

    cm_name = MODULE_NAME;
    SoftwareInventory *soft_cm1_rest = init_software_inventory(update_service->software_inventory, cm_name.append("-REST"));
    // SoftwareInventory *soft_cm1_rest = init_software_inventory(update_service->software_inventory, "CM1-REST");
    soft_cm1_rest->version = "v1.1";
    soft_cm1_rest->manufacturer = "KETI_BMC_REST";
    soft_cm1_rest->release_date = currentDateTime();
    soft_cm1_rest->lowest_supported_version = "v1.1";

    cm_name = MODULE_NAME;
    SoftwareInventory *soft_cm1_ipmi = init_software_inventory(update_service->software_inventory, cm_name.append("-IPMI"));
    // SoftwareInventory *soft_cm1_ipmi = init_software_inventory(update_service->software_inventory, "CM1-IPMI");
    soft_cm1_ipmi->version = "v1.5";
    soft_cm1_ipmi->manufacturer = "KETI_BMC_IPMI";
    soft_cm1_ipmi->release_date = currentDateTime();
    soft_cm1_ipmi->lowest_supported_version = "v1.5";
  }
  return;
}

// void init_software_inventory(Collection *software_inventory_collection,
// string _id)
SoftwareInventory *init_software_inventory(Collection *software_inventory_collection, string _id) {
  string odata_id = software_inventory_collection->odata.id + "/" + _id;
  SoftwareInventory *software_inventory;

  // if (record_is_exist(odata_id))
  // return;

  software_inventory = new SoftwareInventory(odata_id, _id);
  /**
   * Software Inventory Configuration
   */
  software_inventory->updatable = true;

  software_inventory->status.state = STATUS_STATE_ENABLED;
  software_inventory->status.health = STATUS_HEALTH_OK;

  software_inventory_collection->add_member(software_inventory);
  return software_inventory;
  // return;
}

void init_task_service(TaskService *task_service) {
  string odata_id = task_service->odata.id;

  /**
   * Task Service Configuration
   */
  task_service->name = "Task Service";
  task_service->id = "TaskService";
  task_service->service_enabled = true;
  task_service->datetime = currentDateTime();

  task_service->status.state = STATUS_STATE_ENABLED;
  task_service->status.health = STATUS_HEALTH_OK;

  if (!record_is_exist(odata_id + "/Tasks")) {
    task_service->task_collection =
        new Collection(odata_id + "/Tasks", ODATA_TASK_COLLECTION_TYPE);
    task_service->task_collection->name = "Task Collection";
  }
  return;
}

void init_event_service(EventService *event_service) {
  string odata_id = event_service->odata.id;

  /**
   * event_service configuration
   */
  event_service->service_enabled = true;
  event_service->delivery_retry_attempts = 3;
  event_service->delivery_retry_interval_seconds = 60;

  // event_service->sse.event_type = false;
  event_service->sse.metric_report_definition = false;
  event_service->sse.registry_prefix = false;
  event_service->sse.resource_type = false;
  event_service->sse.event_format_type = false;
  event_service->sse.message_id = false;
  event_service->sse.origin_resource = false;
  event_service->sse.subordinate_resources = false;

  event_service->status.state = STATUS_STATE_ENABLED;
  event_service->status.health = STATUS_HEALTH_OK;

  event_service->smtp.smtp_smtp_enabled = false;
  event_service->smtp.smtp_server = "smtp.gmail.com";
  event_service->smtp.smtp_port = 587;
  event_service->smtp.smtp_username = "myketimail555";
  event_service->smtp.smtp_password = "";
  event_service->smtp.smtp_sender_address = "myketimail555@gmail.com";

  if (!record_is_exist(odata_id + "/Subscriptions")) {
    event_service->subscriptions = new Collection(
        odata_id + "/Subscriptions", ODATA_EVENT_DESTINATION_COLLECTION_TYPE);
    event_service->subscriptions->name = "Subscription Collection";

    // init_event_destination(event_service->subscriptions, to_string(allocate_numset_num(ALLOCATE_SUBSCRIPTION_NUM)));
  }
  return;
}

void init_event_destination(Collection *event_destination_collection,
                            string _id) {
  string odata_id = event_destination_collection->odata.id + "/" + _id;
  EventDestination *event_destination;

  if (record_is_exist(odata_id))
    return;

  event_destination = new EventDestination(odata_id, _id);

  /**
   * Event Destination Configuration
   */
  event_destination->subscription_type = "RedfishEvent";
  event_destination->delivery_retry_policy = "SuspendRetries";
  event_destination->protocol = "Redfish";

  event_destination->status.state = STATUS_STATE_ENABLED;
  event_destination->status.health = STATUS_HEALTH_OK;

  event_destination_collection->add_member(event_destination);
  return;
}

void init_account_service(AccountService *account_service) {
  string odata_id = account_service->odata.id;

  // AccountService Configuration
  account_service->name = "Account Service";
  account_service->id = "AccountService";
  account_service->status.state = STATUS_STATE_ENABLED;
  account_service->status.health = STATUS_HEALTH_OK;
  account_service->service_enabled = true;
  account_service->auth_failure_logging_threshold = 0;
  account_service->min_password_length = 6;
  account_service->max_password_length = 24;
  account_service->account_lockout_threshold = 0;
  account_service->account_lockout_duration = 0;
  account_service->account_lockout_counter_reset_after = 0;
  account_service->account_lockout_counter_reset_enabled = true;

  // LDAP
  account_service->ldap.account_provider_type = "LDAPService";
  account_service->ldap.password_set = false;
  account_service->ldap.service_enabled = false;
  account_service->ldap.port = DEFAULT_LDAP_PORT;
  account_service->ldap.service_addresses.push_back(
      "ldaps://ldap.example.org:636");
  account_service->ldap.authentication.authentication_type =
      "UsernameAndPassword";
  account_service->ldap.authentication.username =
      "cn=Manager, dc=example, dc=org";
  account_service->ldap.authentication.password = "";

  account_service->ldap.ldap_service.search_settings.base_distinguished_names
      .push_back("dc=example");
  account_service->ldap.ldap_service.search_settings.base_distinguished_names
      .push_back("dc=org");
  account_service->ldap.ldap_service.search_settings.group_name_attribute = "";
  account_service->ldap.ldap_service.search_settings.groups_attribute =
      "memberof";
  account_service->ldap.ldap_service.search_settings.user_name_attribute =
      "uid";

  // Active Directory
  account_service->active_directory.account_provider_type =
      "ActiveDirectoryService";
  account_service->active_directory.service_enabled = false;
  account_service->active_directory.port = DEFAULT_AD_PORT;
  account_service->active_directory.service_addresses.push_back(
      "ad1.example.org");
  // account_service->active_directory.service_addresses.push_back(
  //     "ad2.example.org");

  account_service->active_directory.authentication.authentication_type =
      "UsernameAndPassword";
  account_service->active_directory.authentication.username = "Administrators";

  if (!record_is_exist(odata_id + "/Roles")) {
    account_service->role_collection =
        new Collection(odata_id + "/Roles", ODATA_ROLE_COLLECTION_TYPE);
    account_service->role_collection->name = "Roles Collection";

    string role_odata = account_service->role_collection->odata.id;
    // Administrator role configuration
    Role *_administrator =
        new Role(role_odata + "/Administrator", "Administrator");
    _administrator->id = "Administrator";
    _administrator->name = "User Role";
    _administrator->is_predefined = true;
    _administrator->assigned_privileges.push_back("Login");
    _administrator->assigned_privileges.push_back("ConfigureManager");
    _administrator->assigned_privileges.push_back("ConfigureUsers");
    _administrator->assigned_privileges.push_back("ConfigureSelf");
    _administrator->assigned_privileges.push_back("ConfigureComponents");
    account_service->role_collection->add_member(_administrator);

    // Operator role configuration
    Role *_operator = new Role(role_odata + "/Operator", "Operator");
    _operator->id = "Operator";
    _operator->name = "User Role";
    _operator->is_predefined = true;
    _operator->assigned_privileges.push_back("Login");
    _operator->assigned_privileges.push_back("ConfigureSelf");
    _operator->assigned_privileges.push_back("ConfigureComponents");
    account_service->role_collection->add_member(_operator);

    // ReadOnly role configuration
    Role *_read_only = new Role(role_odata + "/ReadOnly", "ReadOnly");
    _read_only->id = "ReadOnly";
    _read_only->name = "User Role";
    _read_only->is_predefined = true;
    _read_only->assigned_privileges.push_back("Login");
    _read_only->assigned_privileges.push_back("ConfigureSelf");
    account_service->role_collection->add_member(_read_only);
  }

  if (!record_is_exist(odata_id + "/Accounts")) {
    account_service->account_collection =
        new Collection(odata_id + "/Accounts", ODATA_ACCOUNT_COLLECTION_TYPE);
    account_service->account_collection->name = "Accounts Collection";

    for (int i = 0; i < get_user_cnt(); i++) {
      Ipmiuser *user = &ipmiUser[i];
      string username = user->getUsername();
      string password = user->getUserpassword();
      string roled;
      string account_odata_id =
          account_service->account_collection->odata.id + "/";
      string account_id = to_string(allocate_numset_num(ALLOCATE_ACCOUNT_NUM));
      // string account_id = to_string(i);
      // insert_numset_num(ALLOCATE_ACCOUNT_NUM, i);

      account_odata_id += account_id;

      switch (user->getUserPriv()) {
      case 0x1:
        roled = "Callback";
        break;
      case 0x2:
        roled = "User";
        break;
      case 0x3:
        roled = "Operator";
        break;
      case 0x4:
        roled = "Administrator";
        break;
      case 0xf:
        roled = "Noaccess";
        break;
      default:
        roled = "Unknown";
        break;
      }

      Account *account = new Account(account_odata_id, account_id, roled);
      account->id = account_id;
      account->name = "User Account";
      account->user_name = username;
      account->password = password;
      account->enabled = true;
      account->locked = false;
      account->callin = (int)user->callin;
      account->ipmi = (int)user->ipmi;
      account->link_auth = (int)user->link_auth;
      account->priv = (int)user->priv;
      if (roled == "Noaccess" || roled == "Unknown" || roled == "Callback")
        account->enabled = false;
      account->locked = true;

      string certificate_collection_id = account_odata_id;
      certificate_collection_id += ODATA_CERTIFICATE_ID;
      account->certificates = new Collection(certificate_collection_id,
                                             ODATA_CERTIFICATE_COLLECTION_TYPE);
      account_service->account_collection->add_member(account);
    }
  }
  return;
}

void init_session_service(SessionService *session_service) {
  string odata_id = session_service->odata.id;

  /**
   * Session Service Configuration
   */
  session_service->name = "Session Service";
  session_service->id = "SessionService";
  session_service->status.state = STATUS_STATE_ENABLED;
  session_service->status.health = STATUS_HEALTH_OK;
  session_service->service_enabled = true;
  session_service->session_timeout = 86400; // 30sec to 86400sec

  if (!record_is_exist(odata_id + "/Sessions")) {
    session_service->session_collection =
        new Collection(ODATA_SESSION_ID, ODATA_SESSION_COLLECTION_TYPE);
    session_service->session_collection->name = "Session Collection";
  }
  return;
}

void init_numset(void) {
  for (int i = 0; i < ALLOCATE_NUM_COUNT; i++) {
    set<unsigned int> empty;
    if (i == ALLOCATE_ACCOUNT_NUM)
      numset_num[i] = 0;
    else
      numset_num[i] = 1;
    numset[i] = empty;
  }
}

void init_message_registry(void) {
  
  MessageRegistry *mr = new MessageRegistry(ODATA_MESSAGE_REGISTRY_ID, "Basic.1.0.0");
  mr->name = "Message Registry";
  mr->language = "en";
  mr->registry_prefix = "Basic";
  mr->registry_version = "1.0.0";

  Message_For_Registry Test;
  // Message Test;
  Test.pattern = "Test";
  // Test.id = "Test"; // msg id
  Test.message = "This is test Message!";
  Test.severity = "OK";
  Test.resolution = "None";
  Test.description = "Test Description";
  Test.number_of_args = 0;
  mr->messages.v_msg.push_back(Test);


  Message_For_Registry Create;

  Create.pattern = "ResourceCreated";
  Create.message = "Resource is Created!";
  Create.severity = "OK";
  Create.resolution = "Resource Information is in the message_args";
  Create.description = "Resource Create Notice";
  Create.number_of_args = 1;
  Create.param_types.push_back("string");
  mr->messages.v_msg.push_back(Create);

  Message_For_Registry Remove;

  Remove.pattern = "ResourceRemoved";
  Remove.message = "Resource is Removed!";
  Remove.severity = "OK";
  Remove.resolution = "Resource Information is in the message_args";
  Remove.description = "Resource Remove Notice";
  Remove.number_of_args = 1;
  Remove.param_types.push_back("string");
  mr->messages.v_msg.push_back(Remove);

  Message_For_Registry Update;

  Update.pattern = "ResourceUpdated";
  Update.message = "Resource is Updated!";
  Update.severity = "OK";
  Update.resolution = "Resource Information is in the message_args";
  Update.description = "Resource Update Notice";
  Update.number_of_args = 1;
  Update.param_types.push_back("string");
  mr->messages.v_msg.push_back(Update);

  Message_For_Registry StatusChange_critical;

  StatusChange_critical.pattern = "StatusChangeCritical";
  StatusChange_critical.message = "Status has changed to Critical at Resource. See message_args for information about Resource location";
  StatusChange_critical.severity = "Critical";
  StatusChange_critical.resolution = "Resource Location is in the message_args";
  StatusChange_critical.description = "Indicate Status Change to Critical";
  StatusChange_critical.number_of_args = 1;
  StatusChange_critical.param_types.push_back("string");
  mr->messages.v_msg.push_back(StatusChange_critical);

  Message_For_Registry StatusChange_ok;

  StatusChange_ok.pattern = "StatusChangeOK";
  StatusChange_ok.message = "Status has changed to OK at Resource. See message_args for information about Resource location";
  StatusChange_ok.severity = "OK";
  StatusChange_ok.resolution = "Resource Location is in the message_args";
  StatusChange_ok.description = "Indicate Status Change to OK";
  StatusChange_ok.number_of_args = 1;
  StatusChange_ok.param_types.push_back("string");
  mr->messages.v_msg.push_back(StatusChange_ok);

  Message_For_Registry StatusChange_warning;

  StatusChange_warning.pattern = "StatusChangeWarning";
  StatusChange_warning.message = "Status has changed to Warning at Resource. See message_args for information about Resource location";
  StatusChange_warning.severity = "Warning";
  StatusChange_warning.resolution = "Resource Location is in the message_args";
  StatusChange_warning.description = "Indicate Status Change to Warning";
  StatusChange_warning.number_of_args = 1;
  StatusChange_warning.param_types.push_back("string");
  mr->messages.v_msg.push_back(StatusChange_warning);



  // mr->messages.v_msg.push_back();
}

void init_dbus_check(void){
  string cmd = "ps | grep KETI-DBus | grep -v grep";
  string result = get_popen_string(cmd.c_str());
  if(result == "")
  {
      // cout << " nnn" << endl;
    system("/firmware/KETI-DBus &");
  }
}