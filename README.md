# SalmonTools

This repository contains (or will contain) a suite of tools that are useful for working with Salmon output.  This is the ideal repository for tools that don't quite belong in the Salmon repository itself, but which are too small to warrant their own separate project.  It's nice to have such things collected in one place.  Contributions and pull-requests are welcome!

# Tools

`salmontools` is the main command-line interace for interacting with tools.  Like `samtools`, it uses separate commands to execute separate functionality.  The available commands are:

  * extract-unmapped &mdash;Takes an `unmapped_names.txt` file from a run of Salmon, as well as the original FASTA/FASTQ files from which the unmapped names were generated, and extracts the corresponding reads from the FASTA/FASTQ file.  The results (the read names and sequences) are written to a user-provided output file. 
  * generateDecoyTranscriptome.sh &mdash; Located in the `scripts/` directory, this is a preprocessing script for creating augmented hybrid fasta file for `salmon index`. It consumes genome fasta (one file given through -g), transcriptome fasta (-t) and the annotation (GTF file given through -a) to create a new hybrid fasta file which contains the decoy sequences from the genome, concatenated with the transcriptome (`gentrome.fa`). It runs [mashmap](https://github.com/marbl/MashMap) (path to binary given through -m) to align transcriptome to an exon masked genome, with 80% homology and extracts the mapped genomic interval. It uses `awk` and [bedtools](https://bedtools.readthedocs.io/en/latest/index.html) (path to binary given through -b) to merge the contiguosly mapped interval and extracts decoy sequences from the genome. It also dumps `decoys.txt` file which contains the name/id of the decoy sequences. Both `gentrome.fa` and `decoys.txt` can be used with `salmon index` with `salmon` >=0.14.0.  
  **NOTE:** Salmon version [v1.0](https://github.com/COMBINE-lab/salmon/releases/tag/v1.0.0) can directly index the genome and transcriptome and doesn't mandates to run the `generateDecoyTranscriptome` script, however it's still backward compatible. Please checkout [this](https://combine-lab.github.io/alevin-tutorial/2019/selective-alignment/) tutorial on how to run salmon with full genome + transcriptome without the annotation.

# Salmon in Alignment mode w/ decoy BAM

Salmon by default, if provided with the decoy aware index and `--writeMappings` flag, dumps the reads aligning to decoys with better aligninment score than transcriptomic target. In an atypical situation where the decoy tagged BAM has to be requantified with salmon in alignment mode, salmon can fail. The general recommendation for such scenario is to filter the BAM file for all such decoy alignment before requantifying with salmon. The following command will remove both the decoy target and the decoy alignment from the decoy tagged BAM and makes it compatible to run in alignment mode in salmon.
```
samtools view -h input.bam | grep -v 'XT:A:D\|DS:D' | samtools view -bS > output.sam
```
