import simplejson
from miwlib import protobuf_json
from miwlib import protobuf_json_writer
from miwlib import log_definition_pb2

def json2pb(json_file, fmt_file):
    f = open(json_file,'r')
    json_str = simplejson.loads(f.read())
    f.close()
    msg_pb = log_definition_pb2.logdef()
    json_pb = protobuf_json.json2pb(msg_pb,json_str)

    fo = open(fmt_file,'w')
    fo.write(json_pb.SerializeToString())
    fo.close()
