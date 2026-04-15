<start name="linux_rtc" ram="1M" ld="no">
  <provides>
    <service name="Rtc"/>
  </provides>
</start>

<start name="lx_fs" caps="200" ram="4M" ld="no">
	<provides> <service name="File_system"/> </provides>
	<config>
		<policy label_prefix="snapper_block"  root="/"  writeable="yes"/>
		<policy label_prefix="isomem_block"  root="/"  writeable="yes"/>
    <policy label_prefix="isomem_fs_block"  root="/"  writeable="yes"/>
	</config>
</start>


<start name="snapper_block" ram="1G">
  <binary name="vfs_block"/>
	<provides><service name="Block"/></provides>
	<config>
    <vfs>
      <fs />
		</vfs>
		<policy label_prefix="vfs"
		        file="/snapper_block.raw" block_size="512" writeable="yes"/>
  </config>
  <route>
    <service name="File_system">
      <child name="lx_fs"/>
    </service>
    <any-service> <parent/> </any-service>
  </route>
</start>

<start name="isomem_block" ram="1G">
  <binary name="vfs_block"/>
	<provides><service name="Block"/></provides>
	<config>
    <vfs>
      <fs />
		</vfs>
		<policy label_prefix="isomem"
		        file="/block0.raw" block_size="512" writeable="yes"/>
  </config>
  <route>
    <service name="File_system">
      <child name="lx_fs"/>
    </service>
    <any-service> <parent/> </any-service>
  </route>
</start>

<start name="isomem_fs_block" ram="1G">
  <binary name="vfs_block"/>
	<provides><service name="Block"/></provides>
	<config>
    <vfs>
      <fs />
		</vfs>
		<policy label_prefix="isomem_vfs"
		        file="/isomem_fs_block.raw" block_size="512" writeable="yes"/>
  </config>
  <route>
    <service name="File_system">
      <child name="lx_fs"/>
    </service>
    <any-service> <parent/> </any-service>
  </route>
</start>


<start name="vfs" ram="200M" caps="200">
  <provides><service name="File_system"/></provides>
  <config>
    <vfs>
      <dir name="dev">
        <block name="block" block_buffer_count="128"/>
      </dir>
      <lwext4 block_device="/dev/block" cache_write_back="yes"
              expand_via_io="no" writeable="yes"
              reporting="no"
              external_cache_size="32M" report_cache="yes"/>
    </vfs>
    <policy label_prefix="snapper" root="/" writeable="yes"/>
  </config>
  <route>
    <service name="Block"> <child name="snapper_block"/> </service>
    <any-service> <parent/> <any-child/> </any-service>
  </route>
</start>

<start name="isomem_vfs" ram="200M" caps="200">
  <binary name="vfs"/>
  <provides><service name="File_system"/></provides>
  <config verbose="yes" ld_verbose="yes">
    <vfs>
      <rump fs="ext2fs" ram="10M"/>
    </vfs>
    <policy label_prefix="isomem" root="/" writeable="yes"/>
  </config>
  <route>
    <service name="Block"> <child name="isomem_fs_block"/> </service>
    <any-service> <parent/> <any-child/> </any-service>
  </route>
</start>


<start name="snapper" ram="1G" caps="400">
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
		<service name="File_system"><child name="vfs"/></service>
		<any-service> <parent/> <any-child /> </any-service>
	</route>
</start>

<start name="isomem" caps="5000">
 	<resource name="RAM" quantum="2G"/>
  <config verbose="yes" ld_verbose="yes">
    <vfs>
      <fs/>
    </vfs>
 	</config>
 	<route>
 		<service name="Block"><child name="isomem_block"/></service>
 		<service name="File_system"><child name="isomem_vfs"/></service>    
 		<any-service> <parent/> <any-child /> </any-service>
 	</route>
</start>
