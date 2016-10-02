import sys
from pathlib import *
import argparse
import json

thisFilePath = Path(__file__).resolve()
repoDir = thisFilePath.parent.parent

# TODO: Find MTB in a more sophisticated way.
mtbDir = repoDir.parent / 'mtb'

def main(destFilePath):
  content = {}
  content['folders'] = [{ 'path': str(repoDir) }]
  content['build_systems'] = [
    {
      'name': 'couscous',
      'file_regex': '([A-z]:.*?)\\(([0-9]+)(?:,\\s*[0-9]+)?\\)',

      'variants':
      [
        {
          'name': 'Build Only',
          'windows':
          {
            'cmd': [ str(repoDir/'code'/'build.bat') ],
          },
        },
        {
          'name': 'Build and Run',
          'windows':
          {
            'cmd': [ str(repoDir/'code'/'build.bat'), 'run' ],
          },
        },
        {
          'name': 'Update MTB',
          'windows':
          {
            'cmd': [ 'py', '-3', str(mtbDir/'tools'/'generate_self_contained.py'), '-o', str(repoDir/'code'/'mtb.hpp') ],
          },
        }
      ]
    },
  ]

  destFile = sys.stdout

  if str(destFilePath) != '-':
    if not destFilePath.parent.exists():
      destFilePath.parent.mkdir(parents=True)
    destFile = destFilePath.open('w')

  json.dump(content, destFile, indent=2)

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('dest',
                      nargs='?',
                      type=Path,
                      default=(repoDir / 'workspace' / 'couscous.sublime-project'),
                      help='The destination file.')
  args = parser.parse_args()
  main(destFilePath=args.dest)
