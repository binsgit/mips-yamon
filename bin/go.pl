#! /usr/local/bin/perl -s

if (`uname -s` =~ /sunos/i) 
{
   # We're on a Sun
   $port = "/dev/ecpp0";
} 
elsif (`uname -s` =~ /linux/i) 
{
   # We're on Linux 
   $port = "/dev/lp0";
}
else
{
   print("Unknown OS\n");
   exit;
}

my @flashfiles = @ARGV?@ARGV:glob("yamon*.fl");

if(@flashfiles != 1) {
    if (! -t STDIN) {
	die "FATAL: More than one flash file found in current directory. Run script with explicit flash file as sole argument.\n";
    }
    my $count = 0;
    my %menu;
    print "Please select a flash file by number.\n";
    for (@flashfiles) {
	$count++;
	$menu{$count} = $_;
	printf " %2d %s",$count,`ls -l $_`;
    }
    print "Flash file number (1-$count,^C to quit) > ";
    chomp(my $response = <>);
    if ($response && $menu{$response}) {
	@flashfiles = ($menu{$response});
    } else {
	system("echo '>ABORTED'   > $port" );
	die "FATAL: Invalid response\n";
    }

}
print "INFO: loading @flashfiles ...\n";
system( "cat @flashfiles    > $port" );

