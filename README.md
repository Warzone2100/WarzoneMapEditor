# Warzone 2100 map editor

Editor for levels in Warzone 2100, made in order to deprecate
FlaME and other old tools for maps.

## Project status

Not many people work on map editor so it is pretty incomplete, here is what can be classified as working/not

### What is working?

- Map loading (outsourced to [WMT](https://github.com/maxsupermanhd/WMT))
- Tile rendering
- Initializations and resource managment
- Object browser (GUI)

### What is done but can be improved

- Multi-map support (render many maps at the same time)
- Tileset manager
- Stats browser (GUI)

### What is still in TODO

- PIE parser that will actually support all the models
- PIE models loader (fix reloading of same models twice or more)
- Better texture management
- Tile editing
- Loading/saving maps via GUI
- (eventually) Migrating to [wzmaplib](https://github.com/Warzone2100/warzone2100/tree/master/lib/wzmaplib)
- Mouse interactions

## Authors

- FlexCoral (Maxim Zhuchkov)
- pastdue
- Björn Ali Göransson

## License

GPL-2.0
