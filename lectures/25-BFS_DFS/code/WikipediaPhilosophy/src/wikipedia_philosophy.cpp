#include <algorithm>
#include <iostream>
#include "console.h"
#include "simpio.h"
#include "urlstream.h"
#include "vector.h"
#include "stack.h"
#include "queue.h"

using namespace std;

const string WIKI_PREFIX = "https://en.wikipedia.org";
const string PHILOSOPHY = "/wiki/Philosophy";

struct Vertex {
    string url;
    bool visited;
    Vertex *prev;
    Vertex(string url) {
        this->url = url;
        visited = false;
        prev = NULL;
    }
};

void findPathDFS(string url);
void findPathBFS(string url);
Vector<string> pageLinks(string url);
string removeParens(string s);

int main() {
    while (true) {
        int searchChoice = getInteger("Please select:\n\t1.DFS\n\t2.BFS\n\t","Please enter an integer!");
        string startPage = getLine("Please enter a Wikipedia Page to start on (e.g., Stanford University)");
        // replace space with underscore (necessary for some searches)
        replace( startPage.begin(), startPage.end(), ' ', '_' );

        if (searchChoice == 1) {
            findPathDFS("/wiki/"+startPage);
        } else {
            findPathBFS("/wiki/"+startPage);
        }
        cout << endl;
    }
    return 0;
}

// Perform a DFS on Wikipedia
void findPathDFS(string currentPage) {
    Vector<string> path;
    Vector<string> wikiLinks;
    Stack<Vertex> vertStack;

    int clicks = 1;
    /* algorithm:
           create a stack, s
           s.push(v1)
           while s is not empty:
                v = s.pop()
                if v is Philosophy, stop!
                if v has not been visited:
                     mark v as visited
                     push all neighbors of v onto the stack
     */
    cout << "Starting at: " << currentPage.substr(6) << "->" << endl;
    path.add(currentPage);
    while (currentPage != PHILOSOPHY) {
        wikiLinks = pageLinks(currentPage);
        if (wikiLinks.size() == 0) {
            break; // no links!
        }
        currentPage = wikiLinks[0];
        cout << "             " << currentPage.substr(6);
        if (currentPage == PHILOSOPHY) {
            cout << endl << "Done! (" << clicks << " clicks)" << endl;
        } else {
            cout << "->" << endl;
        }
        // linear search for the link in our path
        // If it exists, we have a recursive loop
        bool recursive = false;
        for (string link : path) {
            if (currentPage == link) {
                recursive = true;
                break;
            }
        }
        path.add(currentPage);
        if (recursive) {
            cout << "Found a recursive path!" << endl;
            break; // didn't find
        }
        clicks++;
    }

}

void findPathBFS(string currentPage) {

}

Vector<string> pageLinks(string url) {
    // url should be in the form of "/wiki/ArticleName"
    iurlstream wikiPage(WIKI_PREFIX+url);
    string page(std::istreambuf_iterator<char>(wikiPage), {});
    // first, get just the body
    page = page.substr(page.find("<body"),page.find("</body>"));

    // next, get everything after "firstHeading" (the title)
    // reason: there could be extra links above this point
    page = page.substr(page.find("id=\"firstHeading\""));

    // next, jump to the first paragraph, so we exclude all the
    // possible extra stuff to redirect, disambiguate, etc.
    page = page.substr(page.find("<p>"));

    // this is somewhat arbitrary, but we're going to ignore everything
    // including and after the "References" section
    page = page.substr(0,page.find("id=\"References\""));

    // finally, take out everything in parenthesis
    page = removeParens(page);

    Vector<string> allLinks = stringSplit(page,"<a href=\"");
    Vector<string> wikiLinks; // just the internal links
    if (allLinks.size() > 0) {
        // start on the second string, because it is the first link
        for (int i=1; i < allLinks.size(); i++) {
            string previous = allLinks[i-1]; // needed to check for parenthesis
            // if the previous ends with an open parenthesis, we want to ignore
            if (previous.size() > 0 and previous[previous.size()-1] != '(') {
                string link = allLinks[i];

                // remove up to quotation mark
                int pos = link.find("\"");
                link = link.substr(0,pos);
                // ignore links that don't start with "/wiki/"
                // and any links that have a colon in them (e.g., "File:")
                if (link.find("/wiki/") == 0
                        and link.find(":") == string::npos) {
                    wikiLinks.add(link);
                }
            }
        }
    }
    return wikiLinks;
}

string removeParens(string s) {
    // this is a _very_ broad function, and has the potential
    // to fail dramatically (e.g., remove most of an article
    // if parentheses aren't matched...
    // Also, this does not remove parentheses inside links,
    // because we might need them

    // first, find a left paren
    size_t leftPos, rightPos;

    leftPos = s.find("(");
    if (leftPos == string::npos) {
        return s; // base case
    }

    // special case: don't remove if preceded by underscore
    if (leftPos != 0 and s[leftPos-1] == '_') {
        // find the corresponding right paren
        rightPos = s.find(")",leftPos);
        return s.substr(0,rightPos+1) + removeParens(s.substr(rightPos+1));
    } else {
        // find a right paren after the left paren

        rightPos = s.find(")",leftPos);
        if (rightPos == string::npos) {
            return s; // not found, do nothing
        }
        string sLeft = s.substr(0,leftPos);
        string sRight = s.substr(rightPos+1);
        return sLeft + removeParens(sRight);
    }
}
