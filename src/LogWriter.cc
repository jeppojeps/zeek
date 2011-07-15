// See the file "COPYING" in the main distribution directory for copyright.

#include "util.h"
#include "LogWriter.h"

namespace bro
{

WriteMessage::~WriteMessage()
	{
	for(int i = 0; i < num_fields; ++i)
		{
			delete vals[i];
		}
		delete[] vals;
	}

const char *LogWriter::Fmt (char * format, ...) const
	{
	va_list args;
	va_start (args, format);
	vsnprintf (strbuf, LOGWRITER_MAX_BUFSZ, format, args);
	va_end (args);
	return strbuf;
	}

void LogWriter::Error (const char *msg) 
	{
	putNotification(new ErrorReport(msg));
	}

LogEmissary::LogEmissary(QueueInterface<MessageEvent *>& push_queue, QueueInterface<MessageEvent *>& pull_queue)
: bound(NULL), push_queue(push_queue), pull_queue(pull_queue), path(""), fields(NULL), num_fields(0) 
	{
	}

LogEmissary::~LogEmissary()
	{
	for(int i = 0; i < num_fields; ++i)
		delete fields[i];
	
	delete [] fields;
	}

void LogEmissary::BindWriter(LogWriter *writer)
{
	bound = writer;
}

bool LogEmissary::Init(string arg_path, int arg_num_fields,
		     LogField* const * arg_fields)
	{
	path = arg_path;
	num_fields = arg_num_fields;
	fields = arg_fields;

	assert(bound);
	push_queue.put(new InitMessage(*bound, arg_path, arg_num_fields, arg_fields));
	
	return true;
	}

bool LogEmissary::Write(int arg_num_fields, LogVal** vals)
	{
	// Double-check that the arguments match. If we get this from remote,
	// something might be mixed up.
	if ( num_fields != arg_num_fields )
		{
		DBG_LOG(DBG_LOGGING, "Number of fields don't match in LogEmissary::Write() (%d vs. %d)",
			arg_num_fields, num_fields);

		return false;
		}

	for ( int i = 0; i < num_fields; ++i )
		{
		if ( vals[i]->type != fields[i]->type )
			{
			DBG_LOG(DBG_LOGGING, "Field type doesn't match in LogEmissary::Write() (%d vs. %d)",
				vals[i]->type, fields[i]->type);
			return false;
			}
		}

	assert(bound);
	push_queue.put(new WriteMessage(*bound, num_fields, fields, vals));

	return true;
	}

bool LogEmissary::SetBuf(bool enabled)
	{
	assert(bound);
	push_queue.put(new BufferMessage(*bound, enabled));
	
	return true;
	}

bool LogEmissary::Rotate(string rotated_path, string postprocessor, double open,
		       double close, bool terminating)
	{
	assert(bound);
	push_queue.put(new RotateMessage(*bound, rotated_path, postprocessor, open, close, terminating));
	if(terminating)
		{
		Finish();
		}
	
	return true;
	}

bool LogEmissary::Flush()
	{
	assert(bound);
	push_queue.put(new FlushMessage(*bound));
	
	return true;
	}

void LogEmissary::Finish()
	{
	assert(bound);
	push_queue.put(new FinishMessage(*bound));
	}

void LogEmissary::DeleteVals(LogVal** vals)
	{
	for ( int i = 0; i < num_fields; i++ )
		delete vals[i];
	delete[] vals;
	}

bool LogWriter::RunPostProcessor(string fname, string postprocessor,
				 string old_name, double open, double close,
				 bool terminating)
	{
	// This function operates in a way that is backwards-compatible with
	// the old Bro log rotation scheme.

	if ( ! postprocessor.size() )
		return true;

	const char* const fmt = "%y-%m-%d_%H.%M.%S";

	struct tm tm1;
	struct tm tm2;

	time_t tt1 = (time_t)open;
	time_t tt2 = (time_t)close;

	localtime_r(&tt1, &tm1);
	localtime_r(&tt2, &tm2);

	char buf1[128];
	char buf2[128];

	strftime(buf1, sizeof(buf1), fmt, &tm1);
	strftime(buf2, sizeof(buf2), fmt, &tm2);

	string cmd = postprocessor;
	cmd += " " + fname;
	cmd += " " + old_name;
	cmd += " " + string(buf1);
	cmd += " " + string(buf2);
	cmd += " " + string(terminating ? "1" : "0");
	cmd += " &";

	system(cmd.c_str());

	return true;
	}

}

