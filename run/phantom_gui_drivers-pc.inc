<start name="fb" caps="200" ram="100M">
	<binary name="vesa_fb"/>
  <config width="1024" height="768" buffered="yes" verbose="yes"/>
	<route>
		<service name="Report">  <child name="report_rom"/> </service>
		<service name="Platform"> <child name="platform"/> </service>
		<service name="Capture">  <child name="nitpicker"/> </service>
		<service name="Timer">    <child name="timer"/> </service>
		<service name="IO_PORT">  <parent/> </service>
		<service name="IO_MEM">   <parent/> </service>
		<service name="ROM">      <parent/> </service>
		<service name="PD">       <parent/> </service>
		<service name="CPU">      <parent/> </service>
		<service name="LOG">      <parent/> </service>
	</route>
</start>

<start name="ps2" caps="80" ram="1280K">
	<config/>
	<route>
		<service name="Platform"> <child name="platform"/> </service>
		<service name="Event"> <child name="event_filter" label="ps2"/> </service>
		<service name="Timer">    <child name="timer"/> </service>
		<service name="ROM">      <parent/> </service>
		<service name="PD">       <parent/> </service>
		<service name="CPU">      <parent/> </service>
		<service name="LOG">      <parent/> </service>
	</route>
</start>

<start name="usb" caps="200" ram="16M">
	<binary name="pc_usb_host"/>
	<provides> <service name="Usb"/> </provides>
	<config>
		<report devices="yes"/>
		<policy label_prefix="usb_hid"><device class="0x3"/></policy>
	</config>
	<route>
		<service name="Platform"> <child name="platform"/> </service>
		<service name="Report" label="devices"> <child name="report_rom"/> </service>
		<service name="Timer">    <child name="timer"/> </service>
		<service name="ROM">   <parent/> </service>
		<service name="PD">    <parent/> </service>
		<service name="RM">    <parent/> </service>
		<service name="CPU">   <parent/> </service>
		<service name="LOG">   <parent/> </service>
	</route>
</start>

<start name="usb_hid" caps="180" ram="10M">
	<route>
		<service name="ROM" label="report"> <child name="report_rom"/> </service>
		<service name="ROM">   <parent/> </service>
		<service name="PD">    <parent/> </service>
		<service name="RM">    <parent/> </service>
		<service name="CPU">   <parent/> </service>
		<service name="LOG">   <parent/> </service>
		<service name="Event"> <child name="event_filter" label="usb"/> </service>
		<service name="Usb">   <child name="usb"/> </service>
		<service name="Timer">    <child name="timer"/> </service>
	</route>
</start>

<start name="event_filter" caps="90" ram="1280K">
	<provides> <service name="Event"/> </provides>
	<route>
		<service name="ROM" label="config"> <parent label="event_filter.config"/> </service>
		<service name="Event"> <child name="nitpicker"/> </service>
		<service name="Timer">    <child name="timer"/> </service>
		<service name="ROM">   <parent/> </service>
		<service name="PD">    <parent/> </service>
		<service name="CPU">   <parent/> </service>
		<service name="LOG">   <parent/> </service>
		<service name="Timer"> <parent/> </service>
	</route>
</start>
