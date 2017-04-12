# -*- coding: utf-8 -*-

from setuptools import setup, find_packages

setup(
    name='miwlib',
    version=1.0,
    description='Python package for miw',
    author='Rudy Villeneuve',
    author_email='rudy.villeneuve@soprasteria.com',
    url='https://github.com/soprasteria/cybersecurity-miw',
    packages=find_packages(exclude=('cli'))
)
