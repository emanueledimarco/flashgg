#!/usr/bin/env python 
import os
import glob
import re

def run(cmd):
  print "%s\n\n"%cmd
  os.system(cmd)

def writePreamble(srcfile,treedir):
    srcfile.write("#!/bin/bash\n")
    srcfile.write("ulimit -s unlimited\n")
    srcfile.write("set -e\n")
    srcfile.write("cd %s/src\n"%os.environ['CMSSW_BASE'])
    srcfile.write("export SCRAM_ARCH=%s\n"%os.environ['SCRAM_ARCH'])
    srcfile.write("source /cvmfs/cms.cern.ch/cmsset_default.sh\n")
    srcfile.write("eval `scramv1 runtime -sh`\n")
    srcfile.write("cd %s\n"%treedir)

def writeCondorSub(srcfile,_exec,_queue,_nJobs):
    srcfile.write("executable = %s.sh\n"%_exec)
    srcfile.write("arguments  = $(ProcId)\n")
    srcfile.write("output     = %s.$(ClusterId).$(ProcId).out\n"%_exec)
    srcfile.write("error      = %s.$(ClusterId).$(ProcId).err\n\n"%_exec)
    srcfile.write("+JobFlavour = \"%s\"\n"%_queue)
    srcfile.write("queue %g\n"%_nJobs)

def writeMkSubdirs(srcfile,treedir,samples):
  srcfile.write("cd %s\n"%treedir)
  for s in samples:
    srcfile.write("mkdir -p %s\n"%s)
    srcfile.write("mv output_%s_*.root %s\n"%(s,s))


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def writeSubFiles(treedir,outdir,options):
    # Make directory to store sub files
    if not os.path.isdir(outdir): os.system("mkdir %s"%(outdir))
    if not os.path.isdir("%s/jobs"%(outdir)): os.system("mkdir %s/jobs"%(outdir))
                       
    _jobdir = "%s/jobs"%outdir
    # Remove current job files
    if len(glob.glob("%s/*"%_jobdir)): os.system("rm %s/*"%_jobdir)
  
    _executable = "condor"
    _f = open("%s/%s.sh"%(_jobdir,_executable),"w") # single .sh script split into separate jobs
    writePreamble(_f,treedir)

    # Write details depending on mode
    subdirs = glob.glob("%s/*"%treedir)
    for idx,idir in enumerate(subdirs):
      cmd = "hadd_all.py"
      if "data" in idir: cmd += " --doBigData "
      fnames = glob.glob("%s/*.root"%idir)
      if len(fnames) and 'EGamma' in fnames[0]: cmd += " --doEGamma "
      _f.write("if [ $1 -eq %g ]; then\n"%idx)
      _f.write("cd %s \n"%idir)
      _f.write("%s \n"%cmd)
      _f.write("fi\n")

    # Close .sh file
    _f.close()
    os.system("chmod 775 %s/%s.sh"%(_jobdir,_executable))

    # Condor submission file
    _fsub = open("%s/%s.sub"%(_jobdir,_executable),"w")
    writeCondorSub(_fsub,_executable,options.queue,len(subdirs))
    _fsub.close()
    
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Function for submitting files to batch system
def submitFiles(outdir):
    _jobdir = "%s/jobs"%outdir
    _executable = "condor"
    cmdLine = "cd %s; condor_submit %s.sub; cd %s"%(_jobdir,_executable,os.getcwd())
    run(cmdLine)
    print "  --> Finished submitting files"

def makeSubdirs(treedir,jobdir,options):
  files=os.listdir(treedir)
  samples=[]
  for f in os.listdir(treedir):
    if os.path.isfile(os.path.join(treedir, f)) and ".root" in f:
      dsname = "_".join(f.split("_")[1:-1])
      if f not in samples:
        samples.append(dsname)
  basedir=os.path.basename(treedir.rstrip("/"))
  srcfilename="%s/subdirs_%s.sh"%(jobdir,basedir)
  _fmkdirs = open("%s/subdirs_%s.sh"%(jobdir,basedir),"w")
  writeMkSubdirs(_fmkdirs,treedir,samples)
  if options.dryRun:
    print "Written shell script to create subdirs in ", srcfilename
  else:
    run("source %s"%srcfilename)
    print("create subdirectories in %s. Now give the hadd_condor.py with this as main dir")

if __name__ == "__main__":
    from optparse import OptionParser
    parser = OptionParser(usage="%prog [options] treedir jobdir")
    parser.add_option("-d", "--dryRun",  dest="dryRun",  action="store_true", default=False, help="do not submit jobs, just write the config files");
    parser.add_option("-p", "--prepareSubDirs", dest="subdirs", action="store_true", default=False, help="To parallelize more, first make one subdir per sample");
    parser.add_option("-q", "--queue",   dest="queue",  type="string", default="workday", help="condor queue to be used");
    (options, args) = parser.parse_args()
    
    if len(args)<2:
        raise RuntimeError, "Need to specify at least treedir jobdir"

    jobdir = args[1]
    writeSubFiles(args[0],jobdir,options)
    if options.subdirs:
      print "For a bettr parallelization, make one subdirectory / sample and submit a merge job per sample"
      makeSubdirs(args[0],jobdir,options)
      exit(0)
    if options.dryRun:
        print "Wrote condor files in ",jobdir," but not submitting it. If everything ok, run w/o the --dryRun option. "
    else:
        print "Submitting... "
        submitFiles(jobdir)
