#!/usr/bin/python3
import pystache
import datetime
import os
import re
import argparse

rend = pystache.Renderer()


def parse_args():
    parser = argparse.ArgumentParser(description="Behavior Auto-generator")
    parser.add_argument("name", type=str, help="the name of the behavior")
    parser.add_argument("-d", "--description", type=str, help="the description of the behavior", default="")
    parser.add_argument("-q", "--dryrun", help="Generate Files just in output folder", action='store_true')
    return parser.parse_args()


def get_dict(behav):
    d = {
        'behavior': str(behav['name']),
        'Tbehavior': str(behav['name']).title(),
        'UCbehavior': str(behav['name']).upper(),
        'behavior_description': 'describe me' if len(behav['description']) is 0 else behav['description']
    }
    return d


def render_and_write(directory, renderer, behav, template):
    extention = template.split('.')[1]
    sub_dir = ""
    if extention in ['cpp', 'cc', 'cxx', 'c']:
        sub_dir = 'src/behavior'
    elif extention in ['h', 'hpp']:
        sub_dir = 'include/behavior'
    elif extention in ['cfg']:
        sub_dir = 'cfg'
    elif extention in ['xml', 'txt']:
        sub_dir = '.'
    else:
        sub_dir = 'auto_generated/behavior'

    if sub_dir in ['.', 'cfg']:
        dir_name = directory + os.sep + sub_dir
    else:
        dir_name = directory + os.sep + sub_dir + os.sep + behav['behavior']
    if not os.path.isdir(dir_name):
        os.mkdir(dir_name)

    p = dir_name + os.sep + template.replace('behavior', behav['behavior'])[:-len('.mustache')]
    if os.path.exists(p) and extention not in ['xml', 'txt']:
        print('Nothing Happend, File is Exists => ', p)
    #            return

    with open(p, "w") as f:
        f.write(renderer.render_path('templates' + os.sep + template, behav))
    lines = []

    if extention == 'xml':
        with open('../../../parsian_ai/package.xml', 'r') as fd:
            for line in fd.readlines():
                if len(lines) > 0 and line == lines[-1]:
                    continue
                lines.append(line)
                if '<export>' in line:
                    lines.append('    <nodelet plugin="${prefix}/' + behav['behavior'] + '.xml"/>\n')
        with open('../../../parsian_ai/package.xml', 'w') as fd:
            for line in lines:
                fd.write(line)
    if extention == 'txt':
        with open('../../../parsian_ai/CMakeLists.txt', 'r') as fd:
            for line in fd.readlines():
                if len(lines) > 0 and line == lines[-1]:
                    continue
                lines.append(line)
                if 'BEHAVIORS' in line:
                    lines.append('include(./' + behav['behavior'] + '_CMakeLists.txt)\n')
                if 'generate_dynamic_reconfigure_options(' in line:
                    lines.append('		cfg/' + behav['behavior'] + '.cfg\n')
        with open('../../../parsian_ai/CMakeLists.txt', 'w') as fd:
            for line in lines:
                fd.write(line)


def clean_folder(name):
    path = os.path.join(os.getcwd(), name)
    if os.path.isdir(path):
        [clean_folder(os.path.join(path, f)) for f in os.listdir(path) if os.path.isdir(os.path.join(path, f))]
        [os.remove(os.path.join(path, f)) for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))]
    else:
        os.mkdir(os.getcwd() + os.sep + name)


def generate_actions(renderer, behav, directory):
    # Clean Out Folder
    clean_folder('out')
    clean_folder('out/src')
    clean_folder('out/include')
    clean_folder('out/cfg')
    clean_folder('out/auto_genetated')
    [render_and_write(directory, renderer, behav, t) for t in os.listdir(os.getcwd() + os.sep + 'templates')]


def main():
    args = parse_args()
    behav = \
        {
            'name': args.name,
            'description': args.description
        }
    if args.dryrun:
        generate_actions(rend, get_dict(behav), 'out')
    else:
        generate_actions(rend, get_dict(behav), '../../../parsian_ai')  # TODO : fix


if __name__ == "__main__":
    main()
