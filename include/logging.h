namespace logger
{
	void write(const char* message, ...)
	{
		va_list args;
		char buffer[1024];
		char buffer2[1024];

		va_start(args, message);
		_vsnprintf_s(buffer, sizeof(buffer), message, args);
		va_end(args);

		_snprintf_s(buffer2, sizeof(buffer2), "[" LOGGER_NAME "] %s\n", buffer);

		printf(buffer2);
		OutputDebugStringA(buffer2);
	}

	void write(const wchar_t* message, ...)
	{
		va_list args;
		wchar_t buffer[2048];
		wchar_t buffer2[2048];

		va_start(args, message);
		_vsnwprintf_s(buffer, sizeof(buffer), message, args);
		va_end(args);

		_snwprintf_s(buffer2, sizeof(buffer2), L"[" LOGGER_NAME "] %s\n", buffer);

		wprintf(buffer2);
		OutputDebugStringW(buffer2);
	}
}