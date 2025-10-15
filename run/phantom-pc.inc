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
	</config>
</start>

<start name="acpi" caps="350">
	<resource name="RAM" quantum="4M"/>
	<route>
		<service name="Report"> <child name="report_rom"/> </service>
		<service name="IO_MEM"> <parent/> </service>
		<service name="LOG">    <parent/> </service>
		<service name="PD">     <parent/> </service>
		<service name="RM">     <parent/> </service>
		<service name="CPU">    <parent/> </service>
		<service name="ROM">    <parent/> </service>
	</route>
</start>

<start name="pci_decode" caps="350">
	<resource name="RAM" quantum="2M"/>
	<route>
		<service name="Report">             <child name="report_rom"/> </service>
		<service name="ROM" label="system"> <child name="report_rom"/> </service>
		<service name="IO_MEM"> <parent/> </service>
		<service name="LOG">    <parent/> </service>
		<service name="PD">     <parent/> </service>
		<service name="RM">     <parent/> </service>
		<service name="CPU">    <parent/> </service>
		<service name="ROM">    <parent/> </service>
	</route>
</start>

<start name="platform" caps="100" managing_system="yes">
	<resource name="RAM" quantum="1M"/>
	<provides>
		<service name="Platform"/>
	</provides>
	<route>
		<service name="ROM" label="devices"> <child name="report_rom"/> </service>
		<service name="Report"> <child name="report_rom"/> </service>
		<service name="IRQ">     <parent/> </service>
		<service name="IO_MEM">  <parent/> </service>
		<service name="IO_PORT"> <parent/> </service>
		<service name="ROM">     <parent/> </service>
		<service name="PD">      <parent/> </service>
		<service name="CPU">     <parent/> </service>
		<service name="LOG">     <parent/> </service>
		<service name="Timer">   <child name="timer"/> </service>
	</route>
	<config>
		<report devices="yes"/>
		<policy label="ahci -> "> <pci class="AHCI"/> </policy>
	</config>
</start>

<start name="ahci_report_rom">
	<binary name="report_rom"/>
	<resource name="RAM" quantum="1M"/>
	<provides> <service name="Report"/> <service name="ROM"/> </provides>
	<config verbose="yes"/>
</start>

<start name="ahci">
	<resource name="RAM" quantum="1M" />
	<provides><service name="Block" /></provides>
	<config atapi="no">
		<report ports="yes"/>
		<policy label="isomem -> " device="0" writeable="yes" />
		<policy label="vfs -> " device="1" writeable="yes" />
	</config>
	<route>
		<service name="Report"> <child name="ahci_report_rom"/> </service>
		<any-service> <parent/> <any-child/> </any-service>
	</route>
</start>

<start name="vfs" ram="200M" caps="200">
  <provides><service name="File_system"/></provides>
  <config>
   	<vfs>
   		<dir name="dev">
   			<block name="block" block_buffer_count="128"/>
   		</dir>
      <dir name="snapper">
        <ram/>
      </dir>
   	</vfs>
   	<policy label_prefix="snapper" root="/snapper" writeable="yes"/>
  </config>
</start>

<start name="snapper" ram="400M" caps="100">
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
      expiration="3600">

    <rtc/>

    <vfs>
      <fs/>
    </vfs>
  </config>
	<route>
		<service name="Block"><child name="ahci"/></service>
		<any-service> <parent/> <any-child /> </any-service>
	</route>
</start>

<start name="isomem" caps="2000">
 	<resource name="RAM" quantum="1G"/>
  
 	<config verbose="yes" ld_verbose="yes">
 	</config>
 	<route>
 		<service name="Block"><child name="ahci"/></service>
 		<any-service> <parent/> <any-child /> </any-service>
 	</route>
</start>
