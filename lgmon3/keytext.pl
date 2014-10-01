#!/usr/bin/perl

while(<>) {
	$keytext = $keytext.$_;
}

$keytext =~ /<(KeyTextList)>([\w\W]*?)<\/\1>/i;
$itemlist = $2;

while ($itemlist =~ /<(Item)([\w\W]*?)>([\w\W]*?)<\/\1>/ig) {
	$item = $2;
	$text = $3;

	$text =~ s/\"/\\\"/g;

	$item =~ /key *= *\" *([\w]*?) *\"/i;
	$key = $1;

	if ( length $text > 0 ){
		print "/* ", $key, " */", "\n";
		print "gchar \*s = N_\(\"";
		$count = 0;
		
		while ( $text =~ /(^.*)/mg ){
			$line = $1;
			$next = $';

			if ( $count != 0 ) {
				print "\"\n";
				print "              \"";
			}
			
			if ( ($line eq "") && ($next eq "") ){
				print "parse error.\n";
			}

			if ( $line ne "" ){
				print $line;
			}

			if ( $next =~ /^\n/ ){
				print "\\n";
			}

			$count++;
		}

		print "\"\)\;\n";
	}
}

