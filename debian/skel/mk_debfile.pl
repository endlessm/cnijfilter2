#!/usr/bin/perl

@files = ();
$ppdfiles=`find ../../ppd -name *.ppd -print | xargs -i basename {} | tr -s '\n' ' '`;

`perl -pe 's/PPD_FILES/$ppdfiles/' postinst.skel > ../postinst`;
`perl -pe 's/PPD_FILES/$ppdfiles/' postrm.skel > ../postrm`;
