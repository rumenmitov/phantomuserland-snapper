<start name="drivers" caps="1500" ram="64M" managing_system="yes">
	<binary name="init"/>
	<route>
		<service name="ROM" label="config"> <parent label="drivers.config"/> </service>
		<service name="Timer">   <child name="timer"/> </service>
		<service name="Capture"> <child name="nitpicker"/> </service>
		<service name="Event">   <child name="nitpicker"/> </service>
    <any-service> <parent /> </any-service>
	</route>
</start>
