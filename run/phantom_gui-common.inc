<start name="drivers" caps="1500" ram="64M" managing_system="yes">
	<binary name="init"/>
	<route>
		<service name="ROM" label="config"> <parent label="drivers.config"/> </service>
		<service name="Timer">   <child name="timer"/> </service>
		<service name="Capture"> <child name="nitpicker"/> </service>
		<service name="Event">   <child name="nitpicker"/> </service>
		<any-service> <parent/> </any-service>
	</route>
</start>

<start name="gui_report_rom" ram="1M" caps="100">
  <binary name="report_rom"/>
	<provides> <service name="Report"/> <service name="ROM"/> </provides>
	<config verbose="no">
		<policy label_suffix="status_bar -> focus"          report="nitpicker -> focus"/>
		<policy label_suffix="nitpicker_config -> xray"     report="global_keys_handler -> xray"/>
		<policy label_suffix="global_keys_handler -> hover" report="nitpicker -> hover"/>
		<policy label_suffix="nit_focus -> clicked"         report="nitpicker -> clicked"/>
		<policy label_suffix="nitpicker -> focus"           report="nit_focus -> focus"/>
	</config>
</start>

<start name="nitpicker_config" ram="1M" caps="100">
	<binary name="rom_filter"/>
	<provides> <service name="ROM"/> </provides>
	<config>
		<input name="xray_enabled" rom="xray" node="xray">
			<attribute name="enabled" /> </input>

		<output node="config">
			<attribute name="focus" value="rom"/>
			<inline>
				<report focus="yes" xray="yes" hover="yes" keystate="yes"
				        clicked="yes"/>
				<capture/> <event/>
				<domain name="pointer" layer="1" origin="pointer"
				        content="client" label="no"/>
				<domain name="panel" layer="2"
				        content="client" label="no" hover="always"/>
        <domain name="" layer="3" content="client" label="no" hover="always" focus="click" width="1024" height="768"/>
			</inline>

			<inline>
				<policy label_prefix="pointer"            domain="pointer"/>
				<policy label_prefix="status_bar"         domain="panel"/>
				<policy label_prefix="scout -> launchpad" domain="launchpad"/>
				<default-policy                           domain=""/>

			</inline>
		</output>
	</config>
	<route>
		<service name="ROM" label="xray"> <child name="gui_report_rom"/> </service>
		<any-service> <parent/> </any-service>
	</route>
</start>


<start name="nitpicker" caps="120" ram="1216K">
	<provides>
		<service name="Gui"/> <service name="Capture"/> <service name="Event"/>
	</provides>
	<route>
		<service name="ROM" label="config">
			<child name="nitpicker_config"/> </service>
		<service name="ROM" label="focus">
			<child name="gui_report_rom"/> </service>
		<service name="Report">
			<child name="gui_report_rom"/> </service>
		<any-service> <parent/> <any-child/> </any-service>
	</route>
</start>

<start name="pointer" ram="1M" caps="100">
	<config/>
</start>

<start name="nit_focus" ram="1M" caps="100">
	<config> <default-policy focus="yes"/> </config>
	<route>
		<service name="ROM" label="clicked"> <child name="gui_report_rom"/> </service>
		<service name="Report"> <child name="gui_report_rom"/> </service>
		<any-service> <parent/> </any-service>
	</route>
</start>
