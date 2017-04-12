import os
import tempfile
from subprocess import call
from miwlib.miwlogger import logger
from miwlib import json2pb
#import re

# beware if a key is also a value
def multi_replace(text, wordDict):
    for key in wordDict:
        text = text.replace(key, wordDict[key])
    return text

# class for running one or more MIW jobs
## TODO:
## - calling options and defaults in constructor
class MIWJob:

    def __init__(self,miw_loc,miw_command=''):
        self.miw_loc = miw_loc
        if miw_command:
            self.miw_command = miw_command
        else:
            self.miw_command = '-fnames $fnames -ofname $ofname -format_name $format_files_repo/$logfile -output_format csv -autosplit -merge_results -memory_factor $memfactor'
                
    def run(self,miw_options):
        #print miw_options.keys()
        #pattern = re.compile(r'\b(' + '|'.join(re.escape(key) for key in miw_options.keys()) + r')\b')
        #local_command = pattern.sub(lambda x: miw_options[x.group()], self.miw_command)

        created_tmp = False
        if '$format_files_repo' in miw_options and '$logfile' in miw_options:
            if not os.path.isfile(miw_options['$format_files_repo'] + '/' + miw_options['$logfile'] + '.fmt'):
                temp_fmt = tempfile.mktemp('.fmt')
                json2pb.json2pb(miw_options['$format_files_repo'] + '/' + miw_options['$logfile'] + '.json', temp_fmt)
                miw_options['$format_files_repo'] = os.path.dirname(temp_fmt)
                miw_options['$logfile'] = os.path.splitext(os.path.basename(temp_fmt))[0]
                created_tmp = True
        
        local_command = multi_replace(self.miw_command,miw_options)
        #print 'local_command=',local_command
        logger.debug("MIW job command=%s" % (local_command))
        call_output = call(self.miw_loc + '/miw ' + local_command,shell=True)
        if created_tmp:
            os.remove(temp_fmt)
        if call_output == 0:
            logger.debug('Successfully MIW job %s' % (local_command))
        else:
            logger.error('Failed MIW job call %s' % (local_command))
        return call_output
