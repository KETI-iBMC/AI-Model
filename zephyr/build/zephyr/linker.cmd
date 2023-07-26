MEMORY
{
 SRAM_NC (wx) : ORIGIN = CONFIG_SRAM_NC_BASE_ADDRESS, LENGTH = CONFIG_SRAM_NC_SIZE * 1024
}
 OUTPUT_FORMAT("elf32-littlearm")
_region_min_align = 4;
MEMORY
    {
    FLASH (rx) : ORIGIN = 0x0, LENGTH = (0*1K - 0x0)
    SRAM (wx) : ORIGIN = 0x0, LENGTH = (16384 * 1K - 16 * 1K)
   
   
   
   
   
   
   
   
   
   
   
    IDT_LIST (wx) : ORIGIN = (0x0 + (16384 * 1K - 16 * 1K)), LENGTH = 2K
    }
ENTRY("__start")
SECTIONS
    {
 .rel.plt :
 {
 *(.rel.plt)
 PROVIDE_HIDDEN (__rel_iplt_start = .);
 *(.rel.iplt)
 PROVIDE_HIDDEN (__rel_iplt_end = .);
 }
 .rela.plt :
 {
 *(.rela.plt)
 PROVIDE_HIDDEN (__rela_iplt_start = .);
 *(.rela.iplt)
 PROVIDE_HIDDEN (__rela_iplt_end = .);
 }
 .rel.dyn :
 {
 *(.rel.*)
 }
 .rela.dyn :
 {
 *(.rela.*)
 }
    /DISCARD/ :
 {
 *(.plt)
 }
    /DISCARD/ :
 {
 *(.iplt)
 }
   
 _image_rom_start = 0x0;
    rom_start :
 {
. = 0x0;
. = ALIGN(4);
. = ALIGN( 1 << LOG2CEIL(4 * 32) );
. = ALIGN( 1 << LOG2CEIL(4 * (16 + 240)) );
_vector_start = .;
KEEP(*(.exc_vector_table))
KEEP(*(".exc_vector_table.*"))
KEEP(*(.gnu.linkonce.irq_vector_table*))
KEEP(*(.vectors))
_vector_end = .;
KEEP(*(.sboot))
 } > SRAM
    text :
 {
 _image_text_start = .;
 *(.text)
 *(".text.*")
 *(".TEXT.*")
 *(.gnu.linkonce.t.*)
 *(.glue_7t) *(.glue_7) *(.vfp11_veneer) *(.v4_bx)
 } > SRAM
 _image_text_end = .;
 .ARM.extab :
 {
 *(.ARM.extab* .gnu.linkonce.armextab.*)
 } > SRAM
 .ARM.exidx :
 {
 __exidx_start = .;
 *(.ARM.exidx* gnu.linkonce.armexidx.*)
 __exidx_end = .;
 } > SRAM
 _image_rodata_start = .;
 initlevel :
 {
  __init_start = .;
  __init_PRE_KERNEL_1_start = .; KEEP(*(SORT(.z_init_PRE_KERNEL_1[0-9]_*))); KEEP(*(SORT(.z_init_PRE_KERNEL_1[1-9][0-9]_*)));
  __init_PRE_KERNEL_2_start = .; KEEP(*(SORT(.z_init_PRE_KERNEL_2[0-9]_*))); KEEP(*(SORT(.z_init_PRE_KERNEL_2[1-9][0-9]_*)));
  __init_POST_KERNEL_start = .; KEEP(*(SORT(.z_init_POST_KERNEL[0-9]_*))); KEEP(*(SORT(.z_init_POST_KERNEL[1-9][0-9]_*)));
  __init_APPLICATION_start = .; KEEP(*(SORT(.z_init_APPLICATION[0-9]_*))); KEEP(*(SORT(.z_init_APPLICATION[1-9][0-9]_*)));
  __init_SMP_start = .; KEEP(*(SORT(.z_init_SMP[0-9]_*))); KEEP(*(SORT(.z_init_SMP[1-9][0-9]_*)));
  __init_end = .;
 } > SRAM
 sw_isr_table :
 {
  . = ALIGN(0);
  *(.gnu.linkonce.sw_isr_table*)
 } > SRAM
 initlevel_error :
 {
  KEEP(*(SORT(.z_init_[_A-Z0-9]*)))
 }
 ASSERT(SIZEOF(initlevel_error) == 0, "Undefined initialization levels used.")
 ctors :
 {
  . = ALIGN(4);
  __CTOR_LIST__ = .;
  LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
  KEEP(*(SORT_BY_NAME(".ctors*")))
  LONG(0)
  __CTOR_END__ = .;
 } > SRAM
 init_array :
 {
  . = ALIGN(4);
  __init_array_start = .;
  KEEP(*(SORT_BY_NAME(".init_array*")))
  __init_array_end = .;
 } > SRAM
 app_shmem_regions :
 {
  __app_shmem_regions_start = .;
  KEEP(*(SORT(.app_regions.*)));
  __app_shmem_regions_end = .;
 } > SRAM
 bt_l2cap_fixed_chan_area : SUBALIGN(4) { _bt_l2cap_fixed_chan_list_start = .; KEEP(*(SORT_BY_NAME(._bt_l2cap_fixed_chan.static.*))); _bt_l2cap_fixed_chan_list_end = .; } > SRAM
 bt_gatt_service_static_area : SUBALIGN(4) { _bt_gatt_service_static_list_start = .; KEEP(*(SORT_BY_NAME(._bt_gatt_service_static.static.*))); _bt_gatt_service_static_list_end = .; } > SRAM
 k_p4wq_initparam_area : SUBALIGN(4) { _k_p4wq_initparam_list_start = .; KEEP(*(SORT_BY_NAME(._k_p4wq_initparam.static.*))); _k_p4wq_initparam_list_end = .; } > SRAM
 log_strings_sections :
 {
  __log_strings_start = .;
  KEEP(*(SORT(.log_strings*)));
  __log_strings_end = .;
 } > SRAM
 log_const_sections :
 {
  __log_const_start = .;
  KEEP(*(SORT(.log_const_*)));
  __log_const_end = .;
 } > SRAM
 log_backends_sections :
 {
  __log_backends_start = .;
  KEEP(*("._log_backend.*"));
  __log_backends_end = .;
 } > SRAM
 shell_area : SUBALIGN(4) { _shell_list_start = .; KEEP(*(SORT_BY_NAME(._shell.static.*))); _shell_list_end = .; } > SRAM
 shell_root_cmds_sections :
 {
  __shell_root_cmds_start = .;
  KEEP(*(SORT(.shell_root_cmd_*)));
  __shell_root_cmds_end = .;
 } > SRAM
 font_entry_sections :
 {
  __font_entry_start = .;
  KEEP(*(SORT_BY_NAME("._cfb_font.*")))
  __font_entry_end = .;
 } > SRAM
 tracing_backend_area : SUBALIGN(4) { _tracing_backend_list_start = .; KEEP(*(SORT_BY_NAME(._tracing_backend.static.*))); _tracing_backend_list_end = .; } > SRAM
 zephyr_dbg_info :
 {
  KEEP(*(".dbg_thread_info"));
 } > SRAM
 device_handles :
 {
  __device_handles_start = .;
  KEEP(*(SORT(.__device_handles_pass2*)));
  __device_handles_end = .;
 } > SRAM
    rodata :
 {
 *(.rodata)
 *(".rodata.*")
 *(.gnu.linkonce.r.*)
 . = ALIGN(4);
 } > SRAM
 .gcc_except_table : ONLY_IF_RO
 {
 *(.gcc_except_table .gcc_except_table.*)
 } > SRAM
 _image_rodata_end = .;
 . = ALIGN(_region_min_align);
 _image_rom_end = .;
   
    /DISCARD/ : {
 *(.got.plt)
 *(.igot.plt)
 *(.got)
 *(.igot)
 }
   
 . = 0x0;
 . = ALIGN(_region_min_align);
 _image_ram_start = .;
.ramfunc :
{
 . = ALIGN(_region_min_align);
 _ramfunc_ram_start = .;
 *(.ramfunc)
 *(".ramfunc.*")
 . = ALIGN(_region_min_align);
 _ramfunc_ram_end = .;
} > SRAM
_ramfunc_ram_size = _ramfunc_ram_end - _ramfunc_ram_start;
_ramfunc_rom_start = LOADADDR(.ramfunc);
.bss_nc (NOLOAD) :
{
 __bss_nc_start__ = .;
 *(.nocache.bss*)
 . = ALIGN(4);
 __bss_nc_end__ = .;
} > SRAM_NC
    datas :
 {
 __data_ram_start = .;
 *(.data)
 *(".data.*")
 *(".kernel.*")
 } > SRAM
    __data_rom_start = LOADADDR(datas);
 devices :
 {
  __device_start = .;
  __device_PRE_KERNEL_1_start = .; KEEP(*(SORT(.z_device_PRE_KERNEL_1[0-9]_*))); KEEP(*(SORT(.z_device_PRE_KERNEL_1[1-9][0-9]_*)));
  __device_PRE_KERNEL_2_start = .; KEEP(*(SORT(.z_device_PRE_KERNEL_2[0-9]_*))); KEEP(*(SORT(.z_device_PRE_KERNEL_2[1-9][0-9]_*)));
  __device_POST_KERNEL_start = .; KEEP(*(SORT(.z_device_POST_KERNEL[0-9]_*))); KEEP(*(SORT(.z_device_POST_KERNEL[1-9][0-9]_*)));
  __device_APPLICATION_start = .; KEEP(*(SORT(.z_device_APPLICATION[0-9]_*))); KEEP(*(SORT(.z_device_APPLICATION[1-9][0-9]_*)));
  __device_SMP_start = .; KEEP(*(SORT(.z_device_SMP[0-9]_*))); KEEP(*(SORT(.z_device_SMP[1-9][0-9]_*)));
  __device_end = .;
 } > SRAM
 initshell :
 {
  __shell_module_start = .;
  KEEP(*(".shell_module_*"));
  __shell_module_end = .;
  __shell_cmd_start = .;
  KEEP(*(".shell_cmd_*"));
  __shell_cmd_end = .;
 } > SRAM
 log_dynamic_sections :
 {
  __log_dynamic_start = .;
  KEEP(*(SORT(.log_dynamic_*)));
  __log_dynamic_end = .;
 } > SRAM
 _static_thread_data_area : SUBALIGN(4) { __static_thread_data_list_start = .; KEEP(*(SORT_BY_NAME(.__static_thread_data.static.*))); __static_thread_data_list_end = .; } > SRAM
 k_timer_area : SUBALIGN(4) { _k_timer_list_start = .; *(SORT_BY_NAME(._k_timer.static.*)); _k_timer_list_end = .; } > SRAM
 k_mem_slab_area : SUBALIGN(4) { _k_mem_slab_list_start = .; *(SORT_BY_NAME(._k_mem_slab.static.*)); _k_mem_slab_list_end = .; } > SRAM
 k_mem_pool_area : SUBALIGN(4) { _k_mem_pool_list_start = .; *(SORT_BY_NAME(._k_mem_pool.static.*)); _k_mem_pool_list_end = .; } > SRAM
 k_heap_area : SUBALIGN(4) { _k_heap_list_start = .; *(SORT_BY_NAME(._k_heap.static.*)); _k_heap_list_end = .; } > SRAM
 k_mutex_area : SUBALIGN(4) { _k_mutex_list_start = .; *(SORT_BY_NAME(._k_mutex.static.*)); _k_mutex_list_end = .; } > SRAM
 k_stack_area : SUBALIGN(4) { _k_stack_list_start = .; *(SORT_BY_NAME(._k_stack.static.*)); _k_stack_list_end = .; } > SRAM
 k_msgq_area : SUBALIGN(4) { _k_msgq_list_start = .; *(SORT_BY_NAME(._k_msgq.static.*)); _k_msgq_list_end = .; } > SRAM
 k_mbox_area : SUBALIGN(4) { _k_mbox_list_start = .; *(SORT_BY_NAME(._k_mbox.static.*)); _k_mbox_list_end = .; } > SRAM
 k_pipe_area : SUBALIGN(4) { _k_pipe_list_start = .; *(SORT_BY_NAME(._k_pipe.static.*)); _k_pipe_list_end = .; } > SRAM
 k_sem_area : SUBALIGN(4) { _k_sem_list_start = .; *(SORT_BY_NAME(._k_sem.static.*)); _k_sem_list_end = .; } > SRAM
 k_queue_area : SUBALIGN(4) { _k_queue_list_start = .; *(SORT_BY_NAME(._k_queue.static.*)); _k_queue_list_end = .; } > SRAM
 k_condvar_area : SUBALIGN(4) { _k_condvar_list_start = .; *(SORT_BY_NAME(._k_condvar.static.*)); _k_condvar_list_end = .; } > SRAM
 _net_buf_pool_area : SUBALIGN(4)
 {
  _net_buf_pool_list = .;
  KEEP(*(SORT_BY_NAME("._net_buf_pool.static.*")))
 } > SRAM
 .gcc_except_table : ONLY_IF_RW
 {
 *(.gcc_except_table .gcc_except_table.*)
 } > SRAM
    __data_ram_end = .;
   bss (NOLOAD) :
 {
        . = ALIGN(4);
 __bss_start = .;
 __kernel_ram_start = .;
 *(.bss)
 *(".bss.*")
 *(COMMON)
 *(".kernel_bss.*")
 __bss_end = ALIGN(4);
 } > SRAM
    noinit (NOLOAD) :
        {
        *(.noinit)
        *(".noinit.*")
 *(".kernel_noinit.*")
        } > SRAM
    _image_ram_end = .;
    _end = .;
    __kernel_ram_end = 0x0 + (16384 * 1K - 16 * 1K);
    __kernel_ram_size = __kernel_ram_end - __kernel_ram_start;
   
/DISCARD/ :
{
 KEEP(*(.irq_info*))
 KEEP(*(.intList*))
}
 .stab 0 : { *(.stab) }
 .stabstr 0 : { *(.stabstr) }
 .stab.excl 0 : { *(.stab.excl) }
 .stab.exclstr 0 : { *(.stab.exclstr) }
 .stab.index 0 : { *(.stab.index) }
 .stab.indexstr 0 : { *(.stab.indexstr) }
 .gnu.build.attributes 0 : { *(.gnu.build.attributes .gnu.build.attributes.*) }
 .comment 0 : { *(.comment) }
 .debug 0 : { *(.debug) }
 .line 0 : { *(.line) }
 .debug_srcinfo 0 : { *(.debug_srcinfo) }
 .debug_sfnames 0 : { *(.debug_sfnames) }
 .debug_aranges 0 : { *(.debug_aranges) }
 .debug_pubnames 0 : { *(.debug_pubnames) }
 .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
 .debug_abbrev 0 : { *(.debug_abbrev) }
 .debug_line 0 : { *(.debug_line .debug_line.* .debug_line_end ) }
 .debug_frame 0 : { *(.debug_frame) }
 .debug_str 0 : { *(.debug_str) }
 .debug_loc 0 : { *(.debug_loc) }
 .debug_macinfo 0 : { *(.debug_macinfo) }
 .debug_weaknames 0 : { *(.debug_weaknames) }
 .debug_funcnames 0 : { *(.debug_funcnames) }
 .debug_typenames 0 : { *(.debug_typenames) }
 .debug_varnames 0 : { *(.debug_varnames) }
 .debug_pubtypes 0 : { *(.debug_pubtypes) }
 .debug_ranges 0 : { *(.debug_ranges) }
 .debug_macro 0 : { *(.debug_macro) }
    /DISCARD/ : { *(.note.GNU-stack) }
    .ARM.attributes 0 :
 {
 KEEP(*(.ARM.attributes))
 KEEP(*(.gnu.attributes))
 }
.last_section (NOLOAD) :
{
} > SRAM
_flash_used = LOADADDR(.last_section) - _image_rom_start;
    }
