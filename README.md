# SalmonTools

This repository contains (or will contain) a suite of tools that are useful for working with Salmon output.  This is the ideal repository for tools that don't quite belong in the Salmon repository itself, but which are too small to warrant their own separate project.  It's nice to have such things collected in one place.  Contributions and pull-requests are welcome!

# Tools

`salmontools` is the main command-line interace for interacting with tools.  Like `samtools`, it uses separate commands to execute separate functionality.  The available commands are:

  * extract-unmapped &mdash;Takes an `unmapped_names.txt` file from a run of Salmon, as well as the original FASTA/FASTQ files from which the unmapped names were generated, and extracts the corresponding reads from the FASTA/FASTQ file.  The results (the read names and sequences) are written to a user-provided output file. 
