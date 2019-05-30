# SalmonTools

This repository contains (or will contain) a suite of tools that are useful for working with Salmon output.  This is the ideal repository for tools that don't quite belong in the Salmon repository itself, but which are too small to warrant their own separate project.  It's nice to have such things collected in one place.  Contributions and pull-requests are welcome!

# Tools

`salmontools` is the main command-line interace for interacting with tools.  Like `samtools`, it uses separate commands to execute separate functionality.  The available commands are:

  * extract-unmapped &mdash;Takes an `unmapped_names.txt` file from a run of Salmon, as well as the original FASTA/FASTQ files from which the unmapped names were generated, and extracts the corresponding reads from the FASTA/FASTQ file.  The results (the read names and sequences) are written to a user-provided output file. 
  * generateDecoyTranscriptome.sh &mdash; Located in the `scripts/` directory, this is a preprocessing script for creating augmented hybrid fasta file for `salmon index`. It consumes genome fasta (one file given through -g), transcriptome fasta (-t) and the annotation (GTF file given through -a) to create a new hybrid fasta file which contains the decoy sequences from the genome, concatenated with the transcriptome (`gentrome.fa`). It runs [mashmap](https://github.com/marbl/MashMap) (path to binary given through -m) to align transcriptome to an exon masked genome, with 80% homology and extracts the mapped genomic interval. It uses `awk` and [bedtools](https://bedtools.readthedocs.io/en/latest/index.html) (path to binary given through -b) to merge the contiguosly mapped interval and extracts decoy sequences from the genome. It also dumps `decoys.txt` file which contains the name/id of the decoy sequences. Both `gentrome.fa` and `decoys.txt` can be used with `salmon index` with `salmon` >=0.14.0
