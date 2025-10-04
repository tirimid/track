// SPDX-License-Identifier: Unlicense

// standard library.
#include	<cstdint>
#include	<cstdio>
#include <cstring>
#include	<filesystem>
#include	<format>
#include	<string>

// system dependencies.
extern "C"
{
#include	<getopt.h>
#include	<sys/time.h>
#include <unistd.h>
}

#define	OUT
#define	IN_OUT
#define	DATA_FILE ".track"

enum Status
{
	NOT_TRACKING = 0,
	WASTE,
	WORKING,
	READING,
	WRITING,
	PROGRAMMING,
	STUDYING
};

struct [[gnu::packed]] Data
{
	uint8_t			m_Status					{};
	uint64_t			m_StartTimestamp		{};
	uint64_t			m_TotalWaste			{};
	uint64_t			m_TotalWorking			{};
	uint64_t			m_TotalReading			{};
	uint64_t			m_TotalWriting			{};
	uint64_t			m_TotalProgramming	{};
	uint64_t			m_TotalStudying		{};
	
	static int32_t	Read(OUT Data& data, FILE* file);
	int32_t			Write(FILE* file);
	void				Update();
	void				Reset();
};

static std::string				FormatTime(uint64_t time, bool precise);
static std::filesystem::path	DataPath();
static void							Usage(char const* programName);
static uint64_t					UnixMicro();

static char const*	statusNames[]	=
{
	"not tracking",
	"wasting time",
	"working",
	"reading",
	"writing",
	"programming",
	"studying"
};

int
main(int argc, char const* argv[])
{
	bool		reset		= false;
	bool		print		= false;
	bool		precise	= false;
	int32_t	c			{};
	while (c = getopt(argc, (char* const*)argv, "hlpr"), c != -1)
	{
		switch (c)
		{
		case 'h':
			Usage(argv[0]);
			return 0;
		case 'l':
			print = true;
			break;
		case 'p':
			precise = true;
			break;
		case 'r':
			reset = true;
			break;
		default:
			return 1;
		}
	}
	
	std::filesystem::path	path	= DataPath();
	
	Data	data	{};
	if (reset)
	{
		FILE*	file	= fopen(path.c_str(), "wb");
		if (!file)
		{
			fprintf(stderr, "err: failed to open data file for writing - %s!\n", path.c_str());
			return 1;
		}
		
		data.Reset();
		
		int32_t	err	= data.Write(file);
		fclose(file);
		if (err)
		{
			return 1;
		}
		
		return 0;
	}
	
	FILE*	file	= fopen(path.c_str(), "rb");
	if (!file)
	{
		fprintf(stderr, "err: failed to open data file for reading - %s!\n", path.c_str());
		return 1;
	}
	
	int32_t	err	= Data::Read(data, file);
	fclose(file);
	if (err)
	{
		return 1;
	}
	
	if (print)
	{
		std::string	waste			= FormatTime(data.m_TotalWaste, precise);
		std::string	working		= FormatTime(data.m_TotalWorking, precise);
		std::string	reading		= FormatTime(data.m_TotalReading, precise);
		std::string	writing		= FormatTime(data.m_TotalWriting, precise);
		std::string	programming	= FormatTime(data.m_TotalProgramming, precise);
		std::string	studying		= FormatTime(data.m_TotalStudying, precise);
		
		uint64_t	total	= 1
			+ data.m_TotalWaste
			+ data.m_TotalWorking
			+ data.m_TotalReading
			+ data.m_TotalWriting
			+ data.m_TotalProgramming
			+ data.m_TotalStudying;
		
		printf(
			"Currently %s\n"
			"\n"
			"Time wasted             %s (%llu%%)\n"
			"Time spent working      %s (%llu%%)\n"
			"Time spent reading      %s (%llu%%)\n"
			"Time spent writing      %s (%llu%%)\n"
			"Time spent programming  %s (%llu%%)\n"
			"Time spent studying     %s (%llu%%)\n",
			statusNames[data.m_Status],
			waste.c_str(),
			(long long unsigned)data.m_TotalWaste / total * 100,
			working.c_str(),
			(long long unsigned)data.m_TotalWorking / total * 100,
			reading.c_str(),
			(long long unsigned)data.m_TotalReading / total * 100,
			writing.c_str(),
			(long long unsigned)data.m_TotalWriting / total * 100,
			programming.c_str(),
			(long long unsigned)data.m_TotalProgramming / total * 100,
			studying.c_str(),
			(long long unsigned)data.m_TotalStudying / total * 100
		);
		
		return 0;
	}
	
	if (optind + 1 != argc)
	{
		fprintf(stderr, "err: missing required positional argument!\n");
		return 1;
	}
	
	file = fopen(path.c_str(), "wb");
	if (!file)
	{
		fprintf(stderr, "err: failed to open data file for writing - %s!\n", path.c_str());
		return 1;
	}
	
	data.Update();
	if (!strcmp(argv[optind], "stop"))
	{
		data.m_Status = NOT_TRACKING;
	}
	else if (!strcmp(argv[optind], "waste"))
	{
		data.m_Status = WASTE;
	}
	else if (!strcmp(argv[optind], "work"))
	{
		data.m_Status = WORKING;
	}
	else if (!strcmp(argv[optind], "read"))
	{
		data.m_Status = READING;
	}
	else if (!strcmp(argv[optind], "write"))
	{
		data.m_Status = WRITING;
	}
	else if (!strcmp(argv[optind], "program"))
	{
		data.m_Status = PROGRAMMING;
	}
	else if (!strcmp(argv[optind], "study"))
	{
		data.m_Status = STUDYING;
	}
	else
	{
		fprintf(stderr, "err: illegal argument - %s!\n", argv[optind]);
		fclose(file);
		return 1;
	}
	
	err = data.Write(file);
	fclose(file);
	if (err)
	{
		fprintf(stderr, "err: failed to write to data file!\n");
		return 1;
	}
}

int32_t
Data::Read(OUT Data& data, FILE* file)
{
	if (fread(&data, sizeof(data), 1, file) != 1)
	{
		fprintf(stderr, "err: failed to read data file!\n");
		return 1;
	}
	return 0;
}

int32_t
Data::Write(FILE* file)
{
	if (fwrite(this, sizeof(*this), 1, file) != 1)
	{
		fprintf(stderr, "err: failed to write data file!\n");
		return 1;
	}
	return 0;
}

void
Data::Update()
{
	uint64_t	end	= UnixMicro();
	uint64_t	delta	= end - m_StartTimestamp;
	
	m_TotalWaste			+= (m_Status == WASTE) * delta;
	m_TotalWorking			+= (m_Status == WORKING) * delta;
	m_TotalReading			+= (m_Status == READING) * delta;
	m_TotalWriting			+= (m_Status == WRITING) * delta;
	m_TotalProgramming	+= (m_Status == PROGRAMMING) * delta;
	m_TotalStudying		+= (m_Status == STUDYING) * delta;
	
	m_StartTimestamp = end;
}

void
Data::Reset()
{
	m_Status					= NOT_TRACKING;
	m_StartTimestamp		= 0;
	m_TotalWaste			= 0;
	m_TotalWorking			= 0;
	m_TotalReading			= 0;
	m_TotalWriting			= 0;
	m_TotalProgramming	= 0;
	m_TotalStudying		= 0;
}

static std::string
FormatTime(uint64_t time, bool precise)
{
	uint64_t	milli		= time / 1000;
	uint64_t	seconds	= milli / 1000;
	uint64_t	minutes	= seconds / 60;
	uint64_t	hours		= minutes / 60;
	
	milli		-= seconds * 1000;
	seconds	-= minutes * 60;
	minutes	-= hours * 60;
	
	if (precise)
	{
		return std::format(
			"{}:{:0>2}:{:0>2}.{:0>3}",
			hours,
			minutes,
			seconds,
			milli
		);
	}
	else
	{
		return std::format("{}:{:0>2}", hours, minutes);
	}
}

static std::filesystem::path
DataPath()
{
	return std::filesystem::path{"/home"} / getlogin() / DATA_FILE;
}

static void
Usage(char const* programName)
{
	fprintf(
		stderr,
		"track: time tracking and productivity utility\n"
		"\n"
		"Usage:\n"
		"\t%s [options] argument\n"
		"\n"
		"Options:\n"
		"\t-h  Display help information and quit\n"
		"\t-l  Display current statistics and quit\n"
		"\t-p  Show precise time information when using -l\n"
		"\t-r  Reset time statistics and quit\n"
		"\n"
		"Accepted arguments:\n"
		"\tstop     Stop tracking time\n"
		"\twaste    Track wasted time\n"
		"\twork     Track time spent working\n"
		"\tread     Track time spent reading\n"
		"\twrite    Track time spent writing\n"
		"\tprogram  Track time spent programming\n"
		"\tstudy    Track time spent studying\n",
		programName
	);
}

static uint64_t
UnixMicro()
{
	timeval	timeData	{};
	gettimeofday(&timeData, nullptr);
	return (uint64_t)timeData.tv_sec * 1000000 + (uint64_t)timeData.tv_usec;
}
