class Contact:
    """ A contact with a first name, a last name, and an email address. """

    def __init__(self, first_name, last_name, email_address):
        """ (Contact, str, str, str) -> NoneType 

        Initialize this Contact with first name first_name, last name 
        last_name, and email address email_address.
        """

        self.first_name = first_name
        self.last_name = last_name
        self.email_address = email_address

if __name__ == "__main__":
    c1 = Contact("Paulo", "Pizarro", "paulo.pizarro@gmail.com")

    print(c1)
