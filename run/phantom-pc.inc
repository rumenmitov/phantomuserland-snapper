<start name="pc_rtc">
  <resource name="RAM" quantum="1M"/>
  <provides>
    <service name="Rtc"/>
  </provides>
</start>

<start name="report_rom">
	<resource name="RAM" quantum="2M"/>
	<provides> <service name="Report"/> <service name="ROM"/> </provides>
	<config>
		<policy label="pci_decode -> system"    report="acpi -> acpi"/>
		<policy label="platform -> devices" report="pci_decode -> devices"/>
		<policy label="usb_hid -> report"    report="usb -> devices"/>
	</config>
</start>

<start name="acpi" caps="350">
	<resource name="RAM" quantum="4M"/>
	<route>
		<service name="Report"> <child name="report_rom"/> </service>
		<any-service> <parent/> <any-child /> </any-service>
	</route>
</start>

<start name="pci_decode" caps="350">
	<resource name="RAM" quantum="2M"/>
	<route>
		<service name="Report">             <child name="report_rom"/> </service>
		<service name="ROM" label="system"> <child name="report_rom"/> </service>
		<any-service> <parent/> <any-child /> </any-service>
	</route>
</start>

<start name="platform" ram="500M" caps="400" managing_system="yes">
	<provides>
		<service name="Platform"/>
	</provides>
	<route>
		<service name="ROM" label="devices"> <child name="report_rom"/> </service>
		<service name="Report"> <child name="report_rom"/> </service>
		<any-service> <parent/> <any-child /> </any-service>
	</route>
	<config>
		<report devices="yes"/>
		<policy label="ahci -> "> <pci class="AHCI"/> </policy>
		<policy label_prefix="ps2"> <device name="ps2"/> </policy>
		<policy label_prefix="fb"  info="yes"> <pci class="VGA"/> </policy>
		<policy label_prefix="usb" info="yes"> <pci class="USB"/> </policy>
	</config>
</start>

<start name="ahci_report_rom">
	<binary name="report_rom"/>
	<resource name="RAM" quantum="1M"/>
	<provides> <service name="Report"/> <service name="ROM"/> </provides>
	<config verbose="yes"/>
</start>

<start name="ahci">
	<resource name="RAM" quantum="20M" />
	<provides><service name="Block" /></provides>
	<config atapi="no">
		<report ports="yes"/>
		<policy label="isomem -> " device="0" writeable="yes" />
		<policy label="snapper_vfs -> " device="1" writeable="yes" />
    <policy label="isomem_vfs -> " device="2" writeable="yes" />
	</config>
	<route>
		<service name="Report"> <child name="ahci_report_rom"/> </service>
		<any-service> <parent/> <any-child/> </any-service>
	</route>
</start>

<start name="snapper_vfs" ram="200M" caps="200">
  <binary name="vfs"/>
  <provides><service name="File_system"/></provides>
  <config>
   	<vfs>
   		<dir name="dev">
   			<block name="block" io_buffer="128"/>
   		</dir>
      <lwext4 block_device="/dev/block" cache_write_back="yes"
              expand_via_io="no" writeable="yes"
              reporting="no"
              external_cache_size="32M" report_cache="yes"/>
   	</vfs>
   	<policy label_prefix="snapper" root="/" writeable="yes"/>
  </config>
</start>

<start name="isomem_vfs" ram="200M" caps="200">
  <binary name="vfs"/>
  <provides><service name="File_system"/></provides>
  <config>
   	<vfs>
      <rump fs="ext2fs" ram="10M"/>
   	</vfs>
   	<policy label_prefix="isomem" root="/" writeable="yes"/>
  </config>
</start>

<start name="snapper" ram="500M" caps="300">
  <provides>
    <service name="Snapper"/>
  </provides>

  <config
      verbose="true"
      redundancy="2"
      integrity="true"
      max_snapshots="0"
      min_snapshots="0"
      threshold="1000"
      expiration="3600"
      bufsize="10M">

    <rtc/>

    <vfs>
      <fs/>
    </vfs>
  </config>
	<route>
		<service name="Block"><child name="ahci"/></service>
		<service name="File_system"><child name="snapper_vfs"/></service>    
		<any-service> <parent/> <any-child /> </any-service>
	</route>
</start>

<start name="isomem" caps="4000">
 	<resource name="RAM" quantum="2G"/>
  
 	<config verbose="yes" ld_verbose="yes">
    <vfs>
      <fs/>
    </vfs>
 	</config>
 	<route>
 		<service name="Block"><child name="ahci"/></service>
		<service name="File_system"><child name="isomem_vfs"/></service>
 		<any-service> <parent/> <any-child /> </any-service>
 	</route>
</start>
