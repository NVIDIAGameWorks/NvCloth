sub GetXPJ
{
	my $xpj = "";
	if ( $^O eq 'linux' )
	{
	  $xpj= "$ENV{P4ROOT_PATH}/physx/buildtools/xpj/1/linux/xpj4";
	}
	elsif ( $^O eq 'darwin' )
	{
	  $xpj= "$ENV{P4ROOT_PATH}/physx/buildtools/xpj/1/osx32/xpj4";
	}
	else
	{
	  $xpj= "$ENV{P4ROOT_PATH}/physx/buildtools/xpj/1/win32/xpj4.exe";
	  $xpj =~ s#/#\\#g;
	}
	return $xpj;
}

sub EnsurePath
{
	my @missed_files;

	foreach (@_)
	{
	push @missed_files, $_ unless (-d $_ or -e $_);
	}

	if ($#missed_files > 0)
	{
	print( "Not all necessaried directories in P4 were synced, please sync the following paths:\n" );
	foreach my $file (@missed_files)
	{
	  print ("\t$file\n");
	}

	die "Missing files";
	}
}

sub GenerateSolutions
{
	my $project = shift;
	my $xpj = GetXPJ();
	if ( $^O eq 'MSWin32' )
	{
	  foreach my $platform (qw(win32 win64))
	  {
			$ENV{PLATFORM} = "windows";
	    foreach my $tool (qw(vc11 vc12 vc14))
	    {
	        print "Generating $tool for $platform ...\n";
	        system( "\"$xpj\" -v 2 -t $tool -p $platform -x $project" );
	    }
	  }

	  foreach my $platform (qw(win32modern win64modern win8arm))
	  {
	    $ENV{PLATFORM} = "windows";
	    foreach my $tool (qw(vc11))
	    {
	        print "Generating $tool for $platform ...\n";
	        system( "\"$xpj\" -v 2 -t $tool -p $platform -x $project" );
	    }
	  }
		
		foreach my $platform (qw(xboxone))
	  {
	    $ENV{PLATFORM} = "xboxone";
	    foreach my $tool (qw(vc11 vc14))
	    {
	        print "Generating $tool for $platform ...\n";
	        system( "\"$xpj\" -v 2 -t $tool -p $platform -x $project" );
	    }
	  }
	  foreach my $platform (qw(ps4))
	  {
		  $ENV{PLATFORM} = "ps4";
	    foreach my $tool (qw(vc10 vc11 vc12 vc14))
	    {
	        print "Generating $tool for $platform ...\n";
	        system( "\"$xpj\" -v 2 -t $tool -p $platform -x $project" );
	    }
	  }
	}
	elsif ( $^O eq 'darwin' )
	{
	  foreach my $platform (qw(osx32 osx64 ios ios64))
	  {
			$ENV{PLATFORM} = "unix";
	    foreach my $tool (qw(make))
	    {
	        print "Generating $tool for $platform ...\n";
	        system( "\"$xpj\" -v 2 -t $tool -p $platform -x $project" );
	    }
	  }
	}
	elsif ( $^O eq 'linux' )
	{
	  foreach my $platform (qw(linux32 linux64))
	  {
			$ENV{PLATFORM} = "unix";
	    foreach my $tool (qw(make))
	    {
	        print "Generating $tool for $platform ...\n";
	        system( "\"$xpj\" -v 2 -t $tool -p $platform -x $project" );
	    }
	  }
	}
}

1;
