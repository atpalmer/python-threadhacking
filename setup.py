from setuptools import setup, Extension


setup(
    name='pthread',
    ext_modules=[
        Extension('pthread', sources=[
            'src/pthread.c',
        ]),
    ],
)
