def MicroMock(**kwargs):
	return type('Object', (), kwargs)()
