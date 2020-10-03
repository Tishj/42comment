/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: tbruinem <tbruinem@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/02 21:30:46 by tbruinem      #+#    #+#                 */
/*   Updated: 2020/10/03 13:07:42 by tbruinem      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

//add option to remove .c|.h check, and instead accept all types of files
//add detection of function, to place the comment outside of the function if found

using namespace std;

enum	e_filedata
{
	COMMENT,
	OTHER,
};

class	FileData
{
	private:
		e_filedata	type;
	public:
		std::string raw;
		e_filedata	getType() {return this->type;}
		FileData(e_filedata type) : type(type) {}
		FileData(e_filedata type, std::string raw) : type(type), raw(raw) {}
		virtual ~FileData() {}
		virtual std::string print() const = 0;
};

class	Other : public FileData
{
	public:
		Other(std::string content) : FileData(OTHER), content(content) {}
		std::string content;
		std::string print() const {return content;}
};

class	Comment : public FileData
{
	public:
		std::vector<std::string>	content;
		Comment(const Comment& orig) : FileData(COMMENT), content(orig.content) {}
		Comment() : FileData(COMMENT) {}
		std::string print() const
		{
			std::string out = "/*\n";
			for (size_t i = 0; i < content.size(); i++)
			{
				out += "**";
				out += '\t';
				out += content[i] + "\n";
			}
			out += "*/";
			return (out);
		}
};

int	usage(void)
{
	cerr << "Usage: 42comment FILE(S)" << endl;
	return (1);
}

bool	correctFileType(string buf)
{
	if (buf.size() <= 2)
		return (false);
	if (buf[buf.size() - 1] != 'c' && buf[buf.size() - 1] != 'h')
		return (false);
	if (buf[buf.size() - 2] != '.')
		return (false);
	return (true);
}

bool	isComment(ifstream& file, string& buf, Comment& comment)
{
	if (buf.size() < 2)
		return (false);
	size_t i = 0;
	for (; i < buf.size() && (buf[i] == '\t' || buf[i] == ' '); i++) {}
	if (buf[i] != '/' && (buf[i] != '*' && buf[i] != '/'))
		return (false);
	i += 2;
	for (; i < buf.size() && (buf[i] == '\t' || buf[i] == ' '); i++) {}
	string content;
	for (; i < buf.size() && buf[i] != '/'; i++)
		content += buf[i];
	if (content.size())
		comment.content.push_back(content);
	while (getline(file, buf))
	{
		if (file.eof() || buf.size() < 2 || (buf[0] == '*' && buf[1] == '/'))
		{
			if (buf.size() >= 2 && (buf[0] == '*' && buf[1] == '/'))
				getline(file, buf);
			break ;
		}
		size_t	j = 0;
		for (; j < buf.size() && (buf[j] == '\t' || buf[j] == ' '); j++) {}
		if (buf[j] != '/' && (buf[j] != '*' && buf[j] != '/'))
			break ;
		for (; j < buf.size() && (buf[j] == '\t' || buf[j] == ' '); j++) {}
		if (!(buf[j] == '*' && buf[j + 1] == '*') && !(buf[j] == '/' && buf[j + 1] == '/'))
			break ;
		content.clear();
		j += 2;
		for (; j < buf.size() && (buf[j] == '\t' || buf[j] == ' '); j++) {}
		for (; j < buf.size() && buf[j] != '/'; j++)
			content += buf[j];
		comment.content.push_back(content);
	}
	return (true);
}

bool	isHeader(ifstream& file, string& buf, vector<FileData*>& filedata)
{
	if (buf.size() < 80 || buf.compare("/* ************************************************************************** */"))
		return (false);
	filedata.push_back(new Other(buf));
	while (getline(file, buf))
	{
		if (buf.compare(0, 2, "/*") || file.eof())
			break ;
		filedata.push_back(new Other(buf));
	}
	return (true);
}

void	parseFile(string fileName)
{
	ifstream	file(fileName.c_str());
	vector<FileData*>	fileContent;
	string buf;
	size_t	lineNumber = 0;
	string	types[] = {
	"COMMENT",
	"OTHER",
	};

	while (getline(file, buf))
	{
		Comment comment;
		if (!lineNumber && isHeader(file, buf, fileContent)) {}
		if (isComment(file, buf, comment))
			fileContent.push_back(new Comment(comment));
		fileContent.push_back(new Other(buf));
		if (file.eof())
			break ;
		lineNumber++;
	}
	file.close();
	e_filedata type = OTHER;
	ofstream	out(fileName.c_str());
	for (size_t i = 0; i < fileContent.size(); i++)
	{
		e_filedata currType = fileContent[i]->getType();
		if (type == COMMENT)
		{
//			cerr << "ADDING NEWLINE" << endl;
			type = currType;
			out << endl;
		}
//		cerr << types[currType] << endl;
		out << fileContent[i]->print() << endl;
		delete fileContent[i];
	}
	out.close();
}

int	main(int argc, char **argv)
{
	if (argc == 1)
		return (usage());
	for (int i = 1; i < argc; i++)
	{
		string fileName = argv[i];
		if (correctFileType(fileName))
			parseFile(fileName);
	}
	return (0);
}
