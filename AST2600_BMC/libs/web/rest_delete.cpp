#include "ipmi/rest_delete.hpp"

bool Ipmiweb_DEL::Del_Usb(json::value request_json){
    //  uint8_t d_response[10000], d_result[10000] = {0}, cmds[300]={0,};
    //   int rets = 0;
    //   memset(d_response, 0, sizeof(uint8_t) * 10000);
    //   memset(cmds, 0, sizeof(uint8_t) * 300);
    //   sprintf(cmds, "lsmod | grep storage");
    //   FILE *p = popen(cmds, "r");
    //   if (p != NULL) {
    //     while (fgets(d_result, sizeof(d_result), p) != NULL)
    //       strncat(d_response, d_result, strlen(d_result));
    //     pclose(p);
    //   }

    //   if (strlen(d_response) > 0) {
    //     rets = system("rmmod g_mass_storage");

    //     if (rets == 0) {
    //       system("umount -l /nfs > /dev/null 2>&1");
    //     }
    //   } else {
    //     memset(cmds, 0, sizeof(300));
    //     sprintf(cmds, "umount -l /nfs > /dev/null 2>&1");
    //     system(cmds);
        
    //   }

    // [테스트] VM 리소스 umount..
    string vm_id;
    get_value_from_json_key(request_json, "ID", vm_id);

    cout << "vm_id : " << vm_id << endl;

    string odata_id = ODATA_MANAGER_ID;
    odata_id.append("/VirtualMedia/").append(vm_id);

    VirtualMedia *vm = (VirtualMedia *)g_record[odata_id];
    string image_path = vm->image_name;
    string check_mounted = "mount | grep " + image_path + " | wc -l";

    if(!(improved_stoi(get_popen_string(check_mounted.c_str())) > 0))
        return false;

    string umount_cmd = "umount /tmp/nfs";
    int ret = system(umount_cmd.c_str());

    if(ret == -1)
        return false;

    vm->inserted = false;
    vm->size = "";
    resource_save_json(vm);

    return true;
    
}

bool Ipmiweb_DEL::Del_User(int index){
  
    app_del_user(index+1);
    
    // [테스트] Account 레드피시로 적용
    // index가 곧 id인 셈이니까 id로 찾아가서 delete해버리면 된다.

    // 해당 index(id)의 계정이 없을때 false
    // 그 다음엔 account collection에서 id 일치하는놈 찾아서 삭제
    string odata_id = ODATA_ACCOUNT_ID;
    odata_id.append("/").append(to_string(index));
    bool exist = false;

    if(!record_is_exist(odata_id))
    {
        log(warning) << "[Web User Delete] : No Account";
        return false;
    }

    Collection *account_col = (Collection *)g_record[ODATA_ACCOUNT_ID];
    std::vector<Resource *>::iterator iter;
    for(iter = account_col->members.begin(); iter != account_col->members.end(); iter++)
    {
        Account *acc = (Account *)*iter;

        if(acc->id == to_string(index))
        {
            // find user
            exist = true;
            break;
        }
    }

    if(exist)
    {
        unsigned int id_num;
        id_num = improved_stoi(((Account *)g_record[odata_id])->id);
        delete_numset_num(ALLOCATE_ACCOUNT_NUM, id_num);

        delete(*iter);
        account_col->members.erase(iter);

        delete_resource(odata_id);
        resource_save_json(account_col);
        return true;
    }
    else
        return false;
    
}


