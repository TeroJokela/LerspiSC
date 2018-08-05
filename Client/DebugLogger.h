#pragma once


class DebugLogger
{
public:
	void writeLog(std::string text)
	{
		std::ofstream f("log.txt", std::ofstream::app);
		if (f.is_open())
		{
			f << text.c_str();
			f << "\n";
			f.close();
		}
	}

	void clearLog()
	{
		// What message could be at the end of the stairs?
	  std
		::
   ofstream
		   f
		    (
	 "log.txt"
		      )
		       ;
     		   if
				 (
				  f
				   .
			  is_open
					 (
					  )
					   )
					    {
						 f
		 				  .
					   close
							(
							 )
							  ;
							   }
								// I want to die
	}
};


extern DebugLogger * oLog;