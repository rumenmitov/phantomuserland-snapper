<start name="drivers" caps="1500" ram="120M" managing_system="yes">
  <binary name="init"/>
  <route>
    <service name="ROM" label="config"> <parent label="drivers.config"/> </service>
    <service name="Timer">   <child name="timer"/> </service>
    <service name="Capture"> <child name="nitpicker"/> </service>
    <service name="Event">   <child name="nitpicker"/> </service>
    <any-service> <parent/> </any-service>
  </route>
</start>

<start name="nitpicker" ram="4M">
	<provides>
		<service name="Gui"/> <service name="Capture"/> <service name="Event"/>
	</provides>
	<config>
		<capture/> <event/>
		<domain name="default" layer="1" content="client" label="no" hover="always" focus="click" width="1024" height="768"/>

		<default-policy domain="default"/>
	</config>
	<route> <any-service> <parent/> <any-child/> </any-service> </route>
</start>
